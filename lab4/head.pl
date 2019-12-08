#!/usr/bin/perl -T
use strict;
use warnings qw(FATAL all);

my $file = shift;
my $count = shift;

print while $_ = shift @ARGV;

print head($file,$count);

sub head {
  my ($file,$count) = @_;
  $count ||= 10;
  open FILE, '<', $file or die "Cannot open ($file) for reading: $!";
  my @data;
  while (defined (my $line = <FILE>)) {
    push @data => $line;
    last if $. >= $count;
  }
  close FILE            or die "Cannot close ($file): $!";
  return @data;
}