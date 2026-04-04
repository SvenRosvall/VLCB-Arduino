#!/bin/env perl

# Generate MarkDown documentation from a header file.
# This script will start generating the documentation from a line with //@Documentation
# Comment lines will be stripped off the comment marker and copied to output.
# Any other line will be copied to output with a bullet marker.

use strict;
my $documenting = 0;
while (<>)
{
    if (/\/\/\@Documentation/) { $documenting = 1; next;}
    if (/\/\/\@End/) { $documenting = 0; next;}
    next if (! $documenting);
    chomp;
    s/\r$//;
    if (/^\s*\/\/\s?/) { print "$'\n"; }
    elsif (/\S/) { s/\);$/\)/; print "* \`$_\` \\\n"; }
    else { print "\n"; }
}
