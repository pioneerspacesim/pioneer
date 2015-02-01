#!/usr/bin/env perl

use 5.010;

use warnings;
use strict;
use autodie qw(:all);

use Date::Format qw(time2str);
use Date::Calc qw(Today Date_to_Time Add_Delta_Days);

use constant SOURCE_DIR            => q{/home/robn/p/pioneer};
use constant TEMP_DIR              => q{/home/robn/p/tmp};
use constant OUT_DIR               => q{/home/robn/p};
use constant CHROOT_DIR            => q{/home/robn/p/wheezy};
use constant THIRDPARTY_DIR        => q{/home/robn/p/pioneer-thirdparty};
use constant MXE_DIR               => q{/home/robn/p/mxe};
use constant CONFIGURE_OPTS        => q{--with-ccache --enable-debug --with-strip};
use constant MAKE_OPTS             => q{-j9};
use constant UPLOAD_PATH           => q{robertnorris,pioneerspacesim@frs.sf.net:/home/frs/project/p/pi/pioneerspacesim};
use constant URL_FORMAT            => q{http://sourceforge.net/projects/pioneerspacesim/files/%s/download};

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
    #osx => {
    #},
};

my ($branch, $platforms) = @ARGV;
$branch //= "master";

my @platforms = grep { exists PLATFORM->{$_} } ($platforms ? (split ',', $platforms) : (keys %{PLATFORM()}));

my ($ref, $date, $buildno) = update_source($branch);
my $version = $branch eq "master" ? $date : $branch;#"$date.$buildno" : $branch;

my %archives;
for my $platform (@platforms) {
    if ($platform ne "osx") {
        my $build_dir = build($platform);
        my $copy_dir = copy($platform, $build_dir);
        if (check($platform, $copy_dir)) {
            $archives{$platform} = archive($platform, $copy_dir);
        } else {
            # should notify someone somehow...?
            say ">>> build $platform failed";
        }
    }
    else {
        # osx, screw it
        system qq{ssh 10.50.0.107 "bash -lc './build-osx.sh $ref $version'"};
        system "scp 10.50.0.107:pioneer/pioneer-$version-osx.tar.bz2 ".OUT_DIR;
        $archives{$platform} = "pioneer-$version-osx.tar.bz2";
    }
}

say STDERR "sleeping for 300 before uploading...";
sleep 300;

upload(values %archives);

tweet(%archives);

tag($version) if $branch eq "master";

sub update_source {
    my ($branch) = @_;

    say STDERR ">>> updating source";

    chdir SOURCE_DIR;
    system "git fetch --tags --all -p";
    system "git checkout origin/$branch || git checkout $branch";

    my $ref = `git log -n1 --pretty=%h origin/$branch`;
    chomp $ref;

    my $date = `date +%Y%m%d`;
    chomp $date;

    my $since = time2str("%Y-%m-%d", Date_to_Time(Add_Delta_Days((Today())[0,1],1,-1),0,0,0));
    my $count = `git rev-list --since=$since --count HEAD`;
    chomp $count;

    return ($ref, $date, $count);
}

sub build_local {
    my ($platform, $configure_opts) = @_;

    say STDERR ">>> local build: ".PLATFORM->{$platform}->{build_dir};

    $configure_opts .= " --with-thirdparty=".PLATFORM->{$platform}->{thirdparty_dir} if PLATFORM->{$platform}->{thirdparty_dir};

    chdir PLATFORM->{$platform}->{build_dir};

    {
        no autodie;
        system "make distclean";
    }
    system "./bootstrap";
    system "./configure $configure_opts";
    system "make clean";
    system "make ".MAKE_OPTS;

    return PLATFORM->{$platform}->{build_dir};
}

sub build_chroot {
    my ($platform, $configure_opts) = @_;

    say STDERR ">>> chroot build";

    system sprintf("sudo rsync -av --delete --exclude=pioneer/.git %s %s/%s", SOURCE_DIR, CHROOT_DIR, $platform);

    $configure_opts .= " --with-thirdparty=/pioneer-thirdparty";
    system sprintf("sudo chroot %s/%s sh -c 'cd pioneer && ./configure $configure_opts && make distclean && ./configure %s && make %s'", CHROOT_DIR, $platform, $configure_opts, MAKE_OPTS);

    # special case for build host, compile the sgm models as well
    if ($platform eq 'linux64') {
        system sprintf("sudo chroot %s/%s sh -c 'cd pioneer && make sgm'", CHROOT_DIR, $platform);
    }

    return sprintf("%s/%s/pioneer", CHROOT_DIR, $platform);
}

sub build {
    my ($platform) = @_;

    say STDERR ">>> $platform: building";

    my @configure_opts = (CONFIGURE_OPTS, PLATFORM->{$platform}->{configure_opts});
    push @configure_opts, "--with-version=$version";
    push @configure_opts, "--with-extra-version=$ref";

    my $configure_opts = join ' ', @configure_opts;

    return PLATFORM->{$platform}->{builder}($platform, $configure_opts);
}

sub filename_base {
    my ($platform) = @_;
    return $branch eq "master" ? "pioneer-$version-$platform" : "pioneer-$version-$ref-$platform";
}

sub copy {
    my ($platform, $source_dir) = @_;

    say ">>> $platform: copying";

    chdir $source_dir;

    my $copy_dir = TEMP_DIR.'/'.filename_base($platform);
    system "mkdir -p $copy_dir";

    my $suffix = PLATFORM->{$platform}->{suffix};

    system "cp -v src/pioneer$suffix $copy_dir";
    system "cp -v src/modelcompiler$suffix $copy_dir";
    system "cp -v src/pioneer.map $copy_dir" if $platform eq 'win32';
    system "cp -v src/modelcompiler.map $copy_dir" if $platform eq 'win32';
    system "cp -v *.txt $copy_dir";
    system "cp -rv licenses $copy_dir/licenses";
    system "cp -rv data $copy_dir/data";

    system "find $copy_dir/data/lang -name cmn.json -delete";
    system "find $copy_dir/data '(' -name .gitignore -o -name Makefile\\\* ')' -delete";

    # if we did a linux64 build we should have sgm files, copy those to the target
    if (-d CHROOT_DIR."/linux64") {
        system sprintf("cd %s/linux64/pioneer && find -name \*.sgm -exec cp -v '{}' %s/'{}' ';'", CHROOT_DIR, $copy_dir);
    }

    return $copy_dir;
}

sub check {
    my ($platform, $binary_dir) = @_;

    say ">>> $platform: checking";

    my $suffix = PLATFORM->{$platform}->{suffix};

    if (! -x "$binary_dir/pioneer$suffix") { return 0; }
    if (! -x "$binary_dir/modelcompiler$suffix") { return 0; }

    return 1;
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
    system join(' ', "scp", @files, UPLOAD_PATH);
}

sub tweet {
    my %archives = @_;

    my $links = join '', map { sprintf " %s ".URL_FORMAT, $_, $archives{$_} } sort keys %archives;

    say STDERR ">>> tweeting";
    system "ttytter -linelength=500 -status='new build available!$links #pioneerspacesim'";

    say STDERR ">>> sending to pusher";
    system "/home/robn/p/pusher-event jameson:builder pioneer 'new build available ($version $ref):$links'";
}

sub tag {
    my ($version) = @_;

    say STDERR ">>> tagging $version";

    chdir SOURCE_DIR;
    system "git tag $version";
    system "git push origin $version";
}
