This readme is a copy of the page http://davekoelle.com/alphanum.html

# The Alphanum Algorithm

People sort strings with numbers differently than software does. 
Most sorting algorithms compare ASCII values, which produces an ordering that is inconsistent with human logic. Here's how to fix it. 

### the available algorithms implmentations :

  * Java: AlphanumComparator.java
  * C#: AlphanumComparator.cs
  * C++: alphanum.cpp
  * C++, not Windows dependent: alphanum.hpp
  * JavaScript: alphanum.js
  * Perl: alphanum.pl
  * Python: alphanum.py
  * Python 2.4+: alphanum.py_v2.4
  * Ruby: alphanum.rb
  * Ocaml: alphanum.oCaml
  * lua : alphanum.lua
  * Groovy : alphanum.groovy
  * PHP: Just use sort(&array, SORT_STRING);
  
License: MIT License - Free to use and distribute

Special thanks to everyone who contributed fixes or new code!

Use at your own risk... I can personally vouch only for the Java version

## The Problem

Look at most sorted list of filenames, product names, or any other text that contains alphanumeric characters - both letters and numbers. Traditional sorting algorithms use ASCII comparisons to sort these items, which means the end-user sees an unfortunately ordered list that does not consider the numeric values within the strings.

For example, in a sorted list of files, "z100.html" is sorted before "z2.html". But obviously, 2 comes before 100!

Sorting algorithms should sort alphanumeric strings in the order that users would expect, especially as software becomes increasingly used by nontechnical people. Besides, it's the 21st Century; software engineers can do better than this.

## The Solution

I created the Alphanum Algorithm to solve this problem. The Alphanum Algorithm sorts strings containing a mix of letters and numbers. Given strings of mixed characters and numbers, it sorts the numbers in value order, while sorting the non-numbers in ASCII order. The end result is a natural sorting order.

Here's a list of sample filenames to illustrate the difference between sorting with the Alphanum algorithm and traditional ASCII sort. On the left is what you live with on a daily basis. On the right is what you could have, if more developers were motivated to sort lists as people would expect. Which list makes more sense to you? Which would be more comfortable to you as you're using an application?

| Traditional | Alphanum |
|---|---|
| z1.doc | z1.doc |
| z10.doc | z2.doc |
| z100.doc | z3.doc |
| z101.doc | z4.doc |
| z102.doc | z5.doc |
| z11.doc | z6.doc |
| z12.doc | z7.doc |
| z13.doc | z8.doc |
| z14.doc | z9.doc |
| z15.doc | z10.doc |
| z16.doc | z11.doc |
| z17.doc | z12.doc |
| z18.doc | z13.doc |
| z19.doc | z14.doc |
| z2.doc | z15.doc |
| z20.doc | z16.doc |
| z3.doc | z17.doc |
| z4.doc | z18.doc |
| z5.doc | z19.doc |
| z6.doc | z20.doc |
| z7.doc | z21.doc |
| z8.doc | z22.doc |
| z9.doc | z23.doc |
          

## How does it work?

The algorithm breaks strings into chunks, where a chunk contains either all alphabetic characters, or all numeric characters. These chunks are then compared against each other. If both chunks contain numbers, a numerical comparison is used. If either chunk contains characters, the ASCII comparison is used.

There is currently a glitch when it comes to periods/decimal points - specifically, periods are treated only as strings, not as decimal points. The solution to this glitch is to recognize a period surrounded by digits as a decimal point, and continue creating a numeric chunck that includes the decimal point. If a letter exists on either side of the period, or if the period is the first or last character in the string, it should be viewed as an actual period and included in an alphabetic chunk. While I have recently figured this out in theory, I have not yet implemented it into the algorithms. To be truly international, the solution shouldn't just consider periods, but should consider whatever decimal separator is used in the current language.

Currently, the algorithm isn't designed to work with negative signs or numbers expressed in scientific notation, like "5*10e-2". In this case, there are 5 chunks: 5, *, 10, e-, and 2.

The latest version of some of the code (particularly the Java version) compares numbers one at a time if those numbers are in chunks of the same size. For example, when comparing abc123 to abc184, 123 and 184 are the same size, so their values are compared digit-by-digit: 1=1, 2<8. This was done to solve the problem of numeric chunks that are too large to fit in range of values allowed by the programming language for a particular datatype: in Java, an int is limited to 2147483647. The problem with this approach is doesn't properly handle numbers that have leading zeros. For example, 0001 is seem as larger than 1 because it's the longer number. A version that does not compare leading zeros is forthcoming.

## Conclusion

Software development has matured beyond the point where simply sorting strings by their ASCII value is acceptable. It is my hope that the Alphanum Algorithm becomes adopted by all developers so we can work together to create software applications that make sense to users. Feel free to download and share the algorithm, place it in your program free of charge, and help spread the word.

## Epilogue: Let's see another example!

Here's an example using fictitious product names. Imagine you're developing an application for a customer, and you need to instill a sense of confidence and professionalism in your product line. Which sorted list would you most associate with those feelings?

| Traditional Sort | Alphanum |
| --- | --- |
| 1000X Radonius Maximus | 10X Radonius |
| 10X Radonius | 20X Radonius |
| 200X Radonius | 20X Radonius Prime |
| 20X Radonius | 30X Radonius |
| 20X Radonius Prime | 40X Radonius |
| 30X Radonius | 200X Radonius |
| 40X Radonius | 1000X Radonius Maximus |
| Allegia 50 Clasteron | Allegia 6R Clasteron |
| Allegia 500 Clasteron | Allegia 50 Clasteron |
| Allegia 50B Clasteron | Allegia 50B Clasteron |
| Allegia 51 Clasteron | Allegia 51 Clasteron |
| Allegia 6R Clasteron | Allegia 500 Clasteron |
| Alpha 100 | Alpha 2 |
| Alpha 2 | Alpha 2A |
| Alpha 200 | Alpha 2A-900 |
| Alpha 2A | Alpha 2A-8000 |
| Alpha 2A-8000 | Alpha 100 |
| Alpha 2A-900 | Alpha 200 |
| Callisto Morphamax | Callisto Morphamax |
| Callisto Morphamax 500 | Callisto Morphamax 500 |
| Callisto Morphamax 5000 | Callisto Morphamax 600 |
| Callisto Morphamax 600 | Callisto Morphamax 700 |
| Callisto Morphamax 6000 SE | Callisto Morphamax 5000 |
| Callisto Morphamax 6000 SE2 | Callisto Morphamax 6000 SE |
| Callisto Morphamax 700 | Callisto Morphamax 6000 SE2 |
| Callisto Morphamax 7000 | Callisto Morphamax 7000 |
| Xiph Xlater 10000 | Xiph Xlater 5 |
| Xiph Xlater 2000 | Xiph Xlater 40 |
| Xiph Xlater 300 | Xiph Xlater 50 |
| Xiph Xlater 40 | Xiph Xlater 58 |
| Xiph Xlater 5 | Xiph Xlater 300 |
| Xiph Xlater 50 | Xiph Xlater 500 |
| Xiph Xlater 500 | Xiph Xlater 2000 |
| Xiph Xlater 5000 | Xiph Xlater 5000 |
| Xiph Xlater 58 | Xiph Xlater 10000 |
     
## Links from Blogs

Even though I wrote this algorithm in 1997 (good thing algorithms are timeless!), it wasn't until December 2007 that this page started to be spread by and talked about on a couple of blogs.

Blogs and sites that have linked to this page, each of which have discussion threads and other links that you may find useful: 

  * [Linked from reddit](http://programming.reddit.com/info/62ppu/comments/)
  * [Linked from Coding Horror](http://www.codinghorror.com/blog/archives/001018.html)
  * [Matías Giovannini implements an alphanumeric sort](http://alaska-kamtchatka.blogspot.com/2007/12/alphanumeric-sort.html)
  * [Linked from Insomnia blog by Francis Beaudet](http://www.surprisedpoultry.com/insomnia/2007/12/user-friendly-sorting.html)
  * [Linked from Ned Batchelder's blog](http://nedbatchelder.com/blog/200712.html#e20071211T054956)
  * [LBrian Huisman realizes natural sorting isn't as easy as you'd think!](http://my.opera.com/GreyWyvern/blog/show.dml/1671288)

