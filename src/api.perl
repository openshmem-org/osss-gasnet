#!/usr/bin/perl
# Emacs: -*- mode: cperl; -*-
use strict;
use warnings;

local $/ = "__API__";

$_ = <>;

while (<>) {
  s/\n/ /g;
  s/ \{.*//;
  print "extern $_;\n";
}
