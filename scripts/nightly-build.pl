#!/usr/bin/env perl

use 5.010;

use warnings;
use strict;
use autodie qw(:all);

use constant SOURCE_DIR         => q{/home/rob/pioneer};
use constant TEMP_DIR           => q{/tmp};
use constant OUT_DIR            => q{/home/rob};
use constant CHROOT_DIR         => q{/home/rob/squeeze};
use constant THIRDPARTY_DIR     => q{/home/rob/pioneer-thirdparty};
use constant MXE_DIR            => q{/home/rob/mxe};
use constant CONFIGURE_OPTS     => q{--with-ccache --enable-debug};
use constant UPLOAD_PATH_FORMAT => q{robertnorris,pioneerspacesim@frs.sf.net:/home/frs/project/p/pi/pioneerspacesim/%s};
use constant URL_FORMAT         => q{http://sourceforge.net/projects/pioneerspacesim/files/nightly/%s/download};

use constant PLATFORM => {
	linux64 => {
		suffix => "",
		configure_opts => "",
        builder => \&build_chroot,
		archiver => \&archive_bz2,
	},
	linux32 => {
		suffix => "",
		configure_opts => "",
        builder => \&build_chroot,
		archiver => \&archive_bz2,
	},
	win32 => {
		suffix => ".exe",
		configure_opts => "--with-mxe=".MXE_DIR,
		build_dir => SOURCE_DIR,
		builder => \&build_local,
		archiver => \&archive_7z,
	},
};

my $branch = shift @ARGV || "master";
my $build_type = shift @ARGV || "nightly";

if (!grep { $_ eq $build_type } qw(alpha nightly)) {
    say STDERR "unknown build type $build_type";
    exit 1;
}

my $ref = update_source($branch);

my %archives;
for my $platform (keys %{PLATFORM()}) {
    my $build_dir = build($platform);
    my $copy_dir = copy($platform, $build_dir);
    $archives{$platform} = archive($platform, $copy_dir);
}

upload(values %archives);

tweet(%archives);

sub update_source {
	my ($branch) = @_;

	say STDERR ">>> updating source";

	chdir SOURCE_DIR;
	system "git fetch";
	system "git checkout origin/$branch || git checkout $branch";

	my $ref = `git log -n1 --pretty=%h origin/$branch`;
    chomp $ref;
    return $ref;
}

sub build_local {
	my ($platform, $configure_opts) = @_;

	say STDERR ">>> local build: ".PLATFORM->{$platform}->{build_dir};

	chdir PLATFORM->{$platform}->{build_dir};

	{
		no autodie;
		system "make distclean";
	}
	system "./bootstrap";
	system "./configure $configure_opts";
	system "make clean";
	system "make";

	return PLATFORM->{$platform}->{build_dir};
}

sub build_chroot {
    my ($platform, $configure_opts) = @_;

    say STDERR ">>> chroot build";

    system sprintf("sudo rsync -av --delete --exclude=pioneer/.git %s %s/%s", SOURCE_DIR, CHROOT_DIR, $platform);
	system sprintf("sudo chroot %s/%s sh -c 'cd pioneer && ./configure && make distclean && ./configure %s && make'", CHROOT_DIR, $platform, $configure_opts);

	return sprintf("%s/%s/pioneer", CHROOT_DIR, $platform);
}

sub build {
	my ($platform) = @_;

	say STDERR ">>> $platform: building";

	my $configure_opts = join ' ', (CONFIGURE_OPTS, PLATFORM->{$platform}->{configure_opts}, "--with-extra-version=$ref");

	return PLATFORM->{$platform}->{builder}($platform, $configure_opts);
}

sub filename_base {
	my ($platform) = @_;

	return
		$build_type eq "alpha" ? "pioneer-$branch-$platform" :
		$branch eq "master"    ? "pioneer-$ref-$platform" :
		                         "pioneer-$branch-$ref-$platform";
}

sub copy {
	my ($platform, $source_dir) = @_;

	say ">>> $platform: copying";

	chdir $source_dir;

	my $copy_dir = TEMP_DIR.'/'.filename_base($platform);
	system "mkdir -p $copy_dir";

	my $suffix = PLATFORM->{$platform}->{suffix};

	system "cp -v src/pioneer$suffix src/modelviewer$suffix $copy_dir";
    system "cp -v src/pioneer.map src/modelviewer.map $copy_dir";
	system "cp -v *.txt $copy_dir";
	system "cp -rv data $copy_dir/data";

    system "find $copy_dir/data '(' -name .gitignore -o -name Makefile\\\* ')' -delete";

	return $copy_dir;
}

sub archive {
	my ($platform, $source_dir) = @_;

	say STDERR ">>> $platform: archiving";

	chdir "$source_dir/..";

	my $archive = PLATFORM->{$platform}->{archiver}($platform);

    system "rm -rf $source_dir";

	return $archive;
}

sub archive_bz2 {
	my ($platform) = @_;

	my $base = filename_base($platform);
	my $out = OUT_DIR."/$base.tar";

    system "tar cvf $out $base";
	system "bzip2 $out";

	my $arc = "$base.tar.bz2";
	say STDERR ">>> $platform: archived to $arc";
	return $arc;
}

sub archive_7z {
	my ($platform) = @_;

	my $base = filename_base($platform);
	my $out = OUT_DIR."/$base.7z";

	system "7za a $out $base";

	my $arc = "$base.7z";
	say STDERR ">>> $platform: archived to $arc";
	return $arc;
}

sub upload {
	my @files = @_;

    chdir OUT_DIR;

	say STDERR ">>> uploading ", join(' ', @files);
	system join(' ', "scp", @files, sprintf(UPLOAD_PATH_FORMAT, $build_type));
}

sub tweet {
	my %archives = @_;

    if ($build_type ne "nightly") {
        say STDERR ">>> not tweeting for $build_type build";
        return;
    }

	say STDERR ">>> tweeting";

	my $links = join '', map { sprintf " %s ".URL_FORMAT, $_, $archives{$_} } sort keys %archives;
	system "ttytter -linelength=500 -status='new dev build available!$links #pioneerspacesim'";
}
