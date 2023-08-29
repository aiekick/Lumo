#
# The Alphanum Algorithm is an improved sorting algorithm for strings
# containing numbers.  Instead of sorting numbers in ASCII order like
# a standard sort, this algorithm sorts numbers in numeric order.
#
# The Alphanum Algorithm is discussed at http://www.DaveKoelle.com
#

# Released under the MIT License - https://opensource.org/licenses/MIT
#
# Copyright 2007-2017 David Koelle
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
# DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
# OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
# USE OR OTHER DEALINGS IN THE SOFTWARE.
#


#
# TODO: Make decimal points be considered in the same class as digits
#

# usage:
#my @sorted = sort { alphanum($a,$b) } @strings;

sub alphanum {
  # split strings into chunks
  my @a = chunkify($_[0]);
  my @b = chunkify($_[1]);

  # while we have chunks to compare.
  while (@a && @b) {
    my $a_chunk = shift @a;
    my $b_chunk = shift @b;

    my $test =
        (($a_chunk =~ /\d/) && ($b_chunk =~ /\d/)) ? # if both are numeric
            $a_chunk <=> $b_chunk : # compare as numbers
            $a_chunk cmp $b_chunk ; # else compare as strings

    # return comparison if not equal.
    return $test if $test != 0;
  }

  # return longer string.
  return @a <=> @b;
}

# split on numeric/non-numeric transitions
sub chunkify {
  my @chunks = split m{ # split on
    (?= # zero width
      (?<=\D)\d | # digit preceded by a non-digit OR
      (?<=\d)\D # non-digit preceded by a digit
    )
  }x, $_[0];
  return @chunks;
}
