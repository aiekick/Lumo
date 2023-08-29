// Released under the MIT License - https://opensource.org/licenses/MIT
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.


//  Contributed by Ted Romaniszyn
//
//  I found your Perl script for a alpha-numeric sort for a job I'm doing right
//  now and have ported it to a MFC class
//  so that I can easily incorporate it into my VC++ application.
//
//  My class is modified version of the CSortStringArray class described in the
//  Microsoft Knowledge Base
//  Article ID: Q120961 "How to Sort a CStringArrary in MFC": <a href="ftp://ftp.microsoft.com/developr/visual_c/kb/q120/9/61.txt">ftp://ftp.microsoft.com/developr/visual_c/kb/q120/9/61.txt</a>
//
//  I am using this new class to sort a list of files selected in a multi-select
//  CFileDialog.  I populate the CSortStringArray object with the selected files
//  and then call the "Sort" method to sort the array.
//
//  Here is the source code for the modified "CompareAndSwap" method that
//  incorporates your improved sorting algorithm.  You can
//  put this code on your WEB page for other people to use.

  BOOL CSortStringArray::CompareAndSwap( int pos )
  {
      // The Alphanum Algorithm is an improved sorting algorithm for strings
      // containing numbers.  Instead of sorting numbers in ASCII order like
      // a standard sort, this algorithm sorts numbers in numeric order.
      //
      // The Alphanum Algorithm is discussed at http://www.DaveKoelle.com
      //
      // This code is a MFC port of the Perl script Alphanum.pl

      static const TCHAR szNumbers[] = _T("0123456789");
      int posFirst = pos;
      int posNext  = pos + 1;

      //Get the strings to compare and make them both lower case
      CString strFirst = GetAt(posFirst);     strFirst.MakeLower();
      CString strNext  = GetAt(posNext);      strNext.MakeLower();

      int n( 0 );
      while ( n == 0 )        //Get next "chunk"
      {
          //A chunk is either a group of letters or a group of numbers
          CString strFirstChunk;
          CString strFirstChar( strFirst[0] );
          if ( strFirstChar.FindOneOf( szNumbers ) != -1 )
          {
              strFirstChunk = strFirst.SpanIncluding( szNumbers );
          }
          else
          {
              strFirstChunk = strFirst.SpanExcluding( szNumbers );
          }

          CString strNextChunk;
          strFirstChar = strNext[0];
          if ( strFirstChar.FindOneOf( szNumbers ) != -1 )
          {
              strNextChunk = strNext.SpanIncluding( szNumbers );
          }
          else
          {
              strNextChunk = strNext.SpanExcluding( szNumbers );
          }

          //Remove the extracted chunks from the strings
          strFirst = strFirst.Mid(strFirstChunk.GetLength(),
                         strFirst.GetLength() - strFirstChunk.GetLength() );
          strNext  = strNext.Mid(strNextChunk.GetLength(),
                         strNext.GetLength() - strNextChunk.GetLength() );

          // Now Compare the chunks
          //# Case 1: They both contain letters
          if ( strFirstChunk.FindOneOf( szNumbers ) == -1 &&
               strNextChunk.FindOneOf( szNumbers )  == -1 )
          {
              n = strFirstChunk.Compare( strNextChunk );
          }
          else
          {
              //Case 2: They both contain numbers
              if ( strFirstChunk.FindOneOf( szNumbers ) != -1 &&
                   strNextChunk.FindOneOf( szNumbers )  != -1)
              {
                  //Convert the numeric strings to long values
                  long lFirst = atol( strFirstChunk );
                  long lNext = atol( strNextChunk );

                  n = 0;
                  if ( lFirst > lNext )
                  {
                      n = 1;
                  }
                  else if ( lFirst < lNext )
                  {
                      n = -1;

                  }
              }
              else
              {
                  // Case 3: One has letters, one has numbers;
                  // or one is empty
                  n = strFirstChunk.Compare( strNextChunk );

                  // If these are equal, make one (which one
                  // is arbitrary) come before
                  // the other   (or else we will be stuck
                  // in this "while n==0" loop)
                  if ( n == 0 )
                  {
                      n = 1;

                  }
              }
          }
      }

      //Now swap the strings in the array
      if ( n != -1 )
      {
          CString temp = GetAt(posFirst);

          SetAt( posFirst, GetAt(posNext) );
          SetAt( posNext, temp );
          return TRUE;
      }
      else
      {
          return FALSE;
      }
  }
