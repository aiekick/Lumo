#
# The Alphanum Algorithm is an improved sorting algorithm for strings
# containing numbers.  Instead of sorting numbers in ASCII order like
# a standard sort, this algorithm sorts numbers in numeric order.
#
# The Alphanum Algorithm is discussed at http://www.DaveKoelle.com
#
#* Python implementation provided by Chris Hulan (chris.hulan@gmail.com)
#* Distributed under same license as original
#
# Released under the MIT License - https://opensource.org/licenses/MIT
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

import re

#
# TODO: Make decimal points be considered in the same class as digits
#

def chunkify(str):
	"""return a list of numbers and non-numeric substrings of +str+

	the numeric substrings are converted to integer, non-numeric are left as is
	"""
	chunks = re.findall("(\d+|\D+)",str)
	chunks = [re.match('\d',x) and int(x) or x for x in chunks] #convert numeric strings to numbers
	return chunks

def alphanum(a,b):
	"""breaks +a+ and +b+ into pieces and returns left-to-right comparison of the pieces

	+a+ and +b+ are expected to be strings (for example file names) with numbers and non-numeric characters
	Split the values into list of numbers and non numeric sub-strings and so comparison of numbers gives
	Numeric sorting, comparison of non-numeric gives Lexicographic order
	"""
	# split strings into chunks
	aChunks = chunkify(a)
	bChunks = chunkify(b)

	return cmp(aChunks,bChunks) #built in comparison works once data is prepared



if __name__ == "__main__":
	unsorted = ["1000X Radonius Maximus","10X Radonius","200X Radonius","20X Radonius","20X Radonius Prime","30X Radonius","40X Radonius","Allegia 50 Clasteron","Allegia 500 Clasteron","Allegia 51 Clasteron","Allegia 51B Clasteron","Allegia 52 Clasteron","Allegia 60 Clasteron","Alpha 100","Alpha 2","Alpha 200","Alpha 2A","Alpha 2A-8000","Alpha 2A-900","Callisto Morphamax","Callisto Morphamax 500","Callisto Morphamax 5000","Callisto Morphamax 600","Callisto Morphamax 700","Callisto Morphamax 7000","Callisto Morphamax 7000 SE","Callisto Morphamax 7000 SE2","QRS-60 Intrinsia Machine","QRS-60F Intrinsia Machine","QRS-62 Intrinsia Machine","QRS-62F Intrinsia Machine","Xiph Xlater 10000","Xiph Xlater 2000","Xiph Xlater 300","Xiph Xlater 40","Xiph Xlater 5","Xiph Xlater 50","Xiph Xlater 500","Xiph Xlater 5000","Xiph Xlater 58"]
	sorted = unsorted[:]
	sorted.sort(alphanum)
	print '+++++Sorted...++++'
	print '\n'.join(sorted)
