#!/usr/bin/env perl

use 5.010;
use warnings;
use strict;
use autodie;

say "usage: prep-changelog <Changelog.txt> <alpha number> <output: html|bbcode>" and exit 1 if @ARGV != 3;
my ($CHANGELOG, $ALPHA, $OUTPUT) = @ARGV;

my ($alpha, @sections, %log);

open my $in, "<", $CHANGELOG;
while (my $line = <$in>) {
    if (my ($n) = $line =~ m/^Alpha (\d+)/) {
        if (not $alpha) {
            if ($n == $ALPHA) {
                $alpha = $n;
            }
            next;
        }
        last;
    }

    next if not $alpha;

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
        say "<h4>Alpha $alpha</h4>";
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

    default {
        say "unknown output: $OUTPUT";
        exit 1;
    }
}
