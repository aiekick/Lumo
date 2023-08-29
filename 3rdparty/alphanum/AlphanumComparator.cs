/*
 * The Alphanum Algorithm is an improved sorting algorithm for strings
 * containing numbers.  Instead of sorting numbers in ASCII order like
 * a standard sort, this algorithm sorts numbers in numeric order.
 *
 * The Alphanum Algorithm is discussed at http://www.DaveKoelle.com
 *
 * Based on the Java implementation of Dave Koelle's Alphanum algorithm.
 * Contributed by Jonathan Ruckwood <jonathan.ruckwood@gmail.com>
 *
 * Adapted by Dominik Hurnaus <dominik.hurnaus@gmail.com> to
 *   - correctly sort words where one word starts with another word
 *   - have slightly better performance
 *
 * Released under the MIT License - https://opensource.org/licenses/MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
using System;
using System.Collections;
using System.Text;

/*
 * Please compare against the latest Java version at http://www.DaveKoelle.com
 * to see the most recent modifications
 */
namespace AlphanumComparator
{
    public class AlphanumComparator : IComparer
    {
        private enum ChunkType {Alphanumeric, Numeric};
        private bool InChunk(char ch, char otherCh)
        {
            ChunkType type = ChunkType.Alphanumeric;

            if (char.IsDigit(otherCh))
            {
                type = ChunkType.Numeric;
            }

            if ((type == ChunkType.Alphanumeric && char.IsDigit(ch))
                || (type == ChunkType.Numeric && !char.IsDigit(ch)))
            {
                return false;
            }

            return true;
        }

        public int Compare(object x, object y)
        {
            String s1 = x as string;
            String s2 = y as string;
            if (s1 == null || s2 == null)
            {
                return 0;
            }

            int thisMarker = 0, thisNumericChunk = 0;
            int thatMarker = 0, thatNumericChunk = 0;

            while ((thisMarker < s1.Length) || (thatMarker < s2.Length))
            {
                if (thisMarker >= s1.Length)
                {
                    return -1;
                }
                else if (thatMarker >= s2.Length)
                {
                    return 1;
                }
                char thisCh = s1[thisMarker];
                char thatCh = s2[thatMarker];

                StringBuilder thisChunk = new StringBuilder();
                StringBuilder thatChunk = new StringBuilder();

                while ((thisMarker < s1.Length) && (thisChunk.Length==0 ||InChunk(thisCh, thisChunk[0])))
                {
                    thisChunk.Append(thisCh);
                    thisMarker++;

                    if (thisMarker < s1.Length)
                    {
                        thisCh = s1[thisMarker];
                    }
                }

                while ((thatMarker < s2.Length) && (thatChunk.Length==0 ||InChunk(thatCh, thatChunk[0])))
                {
                    thatChunk.Append(thatCh);
                    thatMarker++;

                    if (thatMarker < s2.Length)
                    {
                        thatCh = s2[thatMarker];
                    }
                }

                int result = 0;
                // If both chunks contain numeric characters, sort them numerically
                if (char.IsDigit(thisChunk[0]) && char.IsDigit(thatChunk[0]))
                {
                    thisNumericChunk = Convert.ToInt32(thisChunk.ToString());
                    thatNumericChunk = Convert.ToInt32(thatChunk.ToString());

                    if (thisNumericChunk < thatNumericChunk)
                    {
                        result = -1;
                    }

                    if (thisNumericChunk > thatNumericChunk)
                    {
                        result = 1;
                    }
                }
                else
                {
                    result = thisChunk.ToString().CompareTo(thatChunk.ToString());
                }

                if (result != 0)
                {
                    return result;
                }
            }

            return 0;
        }
    }
}
