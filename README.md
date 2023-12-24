# TMBASIC

<!-- See doc/maintainer-instructions.md for instructions on generating this screenshot. -->
<a href="https://tmbasic.com/screenshot.png"><img src="https://tmbasic.com/screenshot.png" alt="Screenshot" class="screenshot" width="500"></a>

<strong>TMBASIC is a [programming language](https://en.wikipedia.org/wiki/Programming_language) for writing [non-graphical programs](https://en.wikipedia.org/wiki/Console_application)</strong>.
The modern BASIC language is easy for beginners to learn.
Experienced programmers will find it familiar and productive after a brief glance at the [cheat sheet](https://tmbasic.com/cheat.html).

<strong>TMBASIC is under development. Stay tuned!</strong>

- [Documentation](https://tmbasic.com/doc.html)<br>
- [Cheat Sheet for Experienced Programmers](https://tmbasic.com/cheat.html)

## It's easy!

**Easy to learn** &mdash; The BASIC language will feel familiar to anyone with programming experience. New programmers will enjoy a cohesive, batteries-included learning experience. Everything you need to develop and distribute sophisticated apps is included in TMBASIC.

**Easy to write** &mdash; Developing a high-quality modern GUI requires professional-level expertise. Text-based apps have few options for visual effects, and even a novice can produce quality results.

**Easy to share** &mdash; TMBASIC apps are standalone executables that require no external library files. They can be copied and opened without installation. TMBASIC can [cross-compile](https://en.wikipedia.org/wiki/Cross_compiler) native executables targeting Windows, macOS, and Linux.

## A taste of TMBASIC

<pre class="taste">
<i>dim</i> a <i>as</i> <i>Number</i>
<i>dim</i> b <i>as</i> <i>String</i>
<i>dim</i> c <i>as</i> <i>Boolean</i>
<i>dim</i> d <b>=</b> <b>1.2345</b>
<i>dim</i> e <b>=</b> <strong>"hello world!"</strong>
<i>dim</i> f <b>=</b> <i>true</i>
</pre>

Declare variables with `dim`, providing either an explicit type or an initial value.
The simple types are:

- `Number` &mdash; [128-bit decimal](https://en.wikipedia.org/wiki/Decimal128_floating-point_format)
- `String` &mdash; [UTF-8 text](https://en.wikipedia.org/wiki/UTF-8)
- `Boolean` &mdash; `true` or `false`

<br><pre class="taste">
<i>dim</i> myList <b>= [1, 2, 3]</b>
<i>dim</i> combinedList <b>=</b> myList <b>+ [4, 5, 6] + 7</b>

<i>dim</i> myMap <i>as Map from Number to String</i>
myMap<b>(123) =</b> <strong>"hello"</strong>
<i>print</i> ContainsKey<b>(</b>myMap<b>, 123)</b>  <u>' true</u>
</pre>

The collection types are:

- `List of T` &mdash; An ordered array of values of type `T`. List indices start at zero.
- `Map from T1 to T2` &mdash; A dictionary with keys of type `T1` and values of type `T2`.
- `Set of T` &mdash; A group of unique values of type `T`.

<br><pre class="taste">
<i>dim list</i> myList
    <i>for</i> i <b>= 1</b> <i>to</i> <b>100</b> <i>step</i> <b>2</b>
        <i>yield</i> i
    <i>next</i>
<i>end dim</i>
</pre>

Use `dim list`, `dim map`, and `dim set` to efficiently build collections.
Each `yield` statement adds an item to the collection.

<br><pre class="taste">
<i>dim</i> d <b>=</b> DateFromParts<b>(2021, 3, 12)</b>
<i>dim</i> dt <b>=</b> DateTimeFromParts<b>(2021, 3, 12, 4, 30, 0, 0)</b>
<i>dim</i> tz <b>=</b> TimeZoneFromName<b>(</b><strong>"America/New_York"</strong><b>)</b>
<i>dim</i> offsets <b>=</b> UtcOffsets<b>(</b>dt<b>,</b> tz<b>)</b>
<i>dim</i> dto <b>=</b> DateTimeOffsetFromParts<b>(2021, 3, 12, 4, 30, 0, 0,</b> offsets<b>(0))</b>
</pre>

The date and time types are:

- `Date` &mdash; A day without a time.
- `DateTime` &mdash; A date and local time without any time zone.
- `DateTimeOffset` &mdash; A date and local time with the time zone offset from UTC.
- `TimeSpan` &mdash; A duration of time.
- `TimeZone` &mdash; A time zone defined by the IANA.

<br><pre class="taste">
<i>type</i> ExampleRecord
    foo <i>as</i> Number
    bar <i>as</i> String
<i>end type</i>

<i>dim</i> myAnonymousRecord1 <i>as Record</i> <b>(</b>foo <i>as Number</i><b>,</b> bar <i>as String</i><b>)</b>
<i>dim</i> myAnonymousRecord2 <b>= {</b> foo<b>: 100,</b> bar<b>:</b> <strong>"hello"</strong> <b>}</b>
<i>dim</i> myNamedRecord <b>=</b> myAnonymousRecord2 <i>as</i> ExampleRecord
</pre>

Both anonymous and named record types are supported.
The literal syntax for anonymous records resembles JavaScript.
Anonymous records can be converted to named records with the `as` operator.

<br><pre class="taste">
<i>dim</i> a <b>= {</b> foo<b>: 1,</b> bar<b>: 2 }</b>
<i>dim</i> b <b>=</b> a    <u>' makes a copy</u>
b<b>.</b>foo <b>= 999</b>  <u>' a.foo is still 1</u>
</pre>

All types, including records and collections, have [value semantics](https://en.wikipedia.org/wiki/Value_semantics).
There are no [references or pointers](https://en.wikipedia.org/wiki/Pointer_(computer_programming)).
Assigning to a variable or passing to a function always makes a copy of the value.

<br><pre class="taste">
<i>sub</i> Main<b>()</b>
    <i>print</i> Squared<b>(5)</b>
<i>end sub</i>

<i>function</i> Squared<b>(</b>x <i>as Number</i><b>)</b> <i>as Number</i>
    <i>return</i> x <i>*</i> x
<i>end function</i>
</pre>

Programs are broken down into functions and subroutines.
A function returns a value to its caller. A subroutine does not return anything.
Program execution begins in the `Main` subroutine.

## Compatibility

<div id="compatibilityTable">

<table><tr><td><strong>Windows</strong><br><small><nobr>Vista or newer</nobr></small></td><td><nobr>64-bit</nobr> &bull; <nobr>32-bit</nobr></td></tr>
<tr><td><strong>macOS</strong><br><small><nobr>10.13 or newer</nobr></small></td><td>Intel &bull; Apple Silicon</td></tr>
<tr><td><strong>Linux</strong><br><small><nobr>2.6.39 or newer</nobr></small></td><td><nobr>64-bit</nobr> &bull; <nobr>32-bit</nobr> &bull; ARM64 &bull; ARM32</td></tr>
<tr><td><strong>iOS</strong></td><td><a href="https://apps.apple.com/us/app/ish-shell/id1436902243">iSH Shell</a> (Linux <nobr>32-bit</nobr>)</td></tr>
<tr><td><strong>Android</strong></td><td><a href="https://termux.com/">Termux</a> (Linux ARM64)</td></tr></table>

</div>

## License
TMBASIC is [open source software](https://en.wikipedia.org/wiki/Open-source_software) that is available free of charge. TMBASIC builds your programs using many open source components. [See a list of components here](https://github.com/electroly/tmbasic/blob/master/doc/third-party-libraries.md). These components have permissive and commercial-friendly licenses that you must obey when you publish your programs written in TMBASIC. When you build your program, TMBASIC produces a `LICENSE.txt` file for your convenience. Include this license file when sharing your TMBASIC apps.
