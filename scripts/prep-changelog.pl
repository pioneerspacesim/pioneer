#!/usr/bin/env perl

use 5.010;
use warnings;
use strict;
use autodie;

say "usage: prep-changelog <Changelog.txt> <version> <output: html|bbcode|markdown>" and exit 1 if @ARGV != 3;
my ($CHANGELOG, $VERSION, $OUTPUT) = @ARGV;

my ($version, @sections, %log);

open my $in, "<", $CHANGELOG;
while (my $line = <$in>) {
    if (my ($n) = $line =~ m/^(\S[\w\s]+\w)/) {
        if (not $version) {
            if ($n eq $VERSION) {
                $version = $n;
            }
            next;
        }
        last;
    }

    next if not $version;

    if (my ($section) = $line =~ m/^   \*\s+(.+?)\s*$/) {
        push @sections, $section;
        next;
    }

    if (my ($log) = $line =~ m/^     \*\s+(.+?)\s*$/) {
        push @{$log{$sections[-1]}}, $log;
        next;
    }

    if (my ($cont) = $line =~ m/^\s+(.+?)\s*$/) {
        $log{$sections[-1]}->[-1] .= " $cont";
        next;
    }
}
close $in;

given ($OUTPUT) {
    when ("html") {
        say "<h4>$version</h4>";
        say "";
        say "<ul>";
        for my $section (@sections) {
            say "  <li>$section</li>";
            say "  <ul>";
            say "    <li>$_</li>" for @{$log{$section}};
            say "  </ul>";
        }
        say "</ul>";
    }

    when ("bbcode") {
        for my $section (@sections) {
            say "[b]$section"."[/b]";
            say "";
            say "[LIST]";
            say "[*]$_"."[/*]" for @{$log{$section}};
            say "[/LIST]";
            say "";
        }
    }

    when ("markdown") {
        for my $section (@sections) {
            say "* $section";
            for my $line (@{$log{$section}}) {
                $line =~ s{#(\d+)}{[#$1](https://github.com/pioneerspacesim/pioneer/issues/$1)}g;
                say "  * $line";
            }
            say "";
        }
    }

    default {
        say "unknown output: $OUTPUT";
        exit 1;
    }
}
