-- alphanum.lua (C) Andre Bogus
--[[ based on the python version of ned batchelder
Distributed under same license as original

Released under the MIT License - https://opensource.org/licenses/MIT

Copyright 2007-2017 David Koelle

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.
]]

-- split a string into a table of number and string values
function splitbynum(s)
       local result = {}
       for x, y in (s or ""):gmatch("(%d*)(%D*)") do
               if x ~= "" then table.insert(result, tonumber(x)) end
               if y ~= "" then table.insert(result, y) end
       end
       return result
end

-- compare two strings
function alnumcomp(x, y)
       local xt, yt = splitbynum(x), splitbynum(y)
       for i = 1, math.min(#xt, #yt) do
               local xe, ye = xt[i], yt[i]
               if type(xe) == "string" then ye = tostring(ye)
               elseif type(ye) == "string" then xe = tostring(xe) end
               if xe ~= ye then return xe < ye end
       end
       return #xt < #yt
end

-- sort a given table of strings the way humans would expect
function sortnicely(t)
               return table.sort(t, alnumcomp)
end

unsorted = {"1000X Radonius Maximus","10X Radonius","200X Radonius","20X Radonius","20X Radonius Prime","30X Radonius","40X Radonius","Allegia 50 Clasteron","Allegia 500 Clasteron","Allegia 51 Clasteron","Allegia 51B Clasteron","Allegia 52 Clasteron","Allegia 60 Clasteron","Alpha 100","Alpha 2","Alpha 200","Alpha 2A","Alpha 2A-8000","Alpha 2A-900","Callisto Morphamax","Callisto Morphamax 500","Callisto Morphamax 5000","Callisto Morphamax 600","Callisto Morphamax 700","Callisto Morphamax 7000","Callisto Morphamax 7000 SE","Callisto Morphamax 7000 SE2","QRS-60 Intrinsia Machine","QRS-60F Intrinsia Machine","QRS-62 Intrinsia Machine","QRS-62F Intrinsia Machine","Xiph Xlater 10000","Xiph Xlater 2000","Xiph Xlater 300","Xiph Xlater 40","Xiph Xlater 5","Xiph Xlater 50","Xiph Xlater 500","Xiph Xlater 5000","Xiph Xlater 58"}

sortnicely(unsorted)
for _, s in ipairs(unsorted) do print(s) end
