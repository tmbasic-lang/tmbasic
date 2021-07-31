#procedure
sub Main()
    print "^"
    print 2^8+1
    print 2*64 ^ 0.5

    print "Abs"
    print Abs(5)
    print Abs(-5)

    print "Sqr"
    print Sqr(9)
    print Sqr(-1)

    print "Cos"
    print Cos(0)
    print Cos(PI)

    print "Sin"
    print Sin(0)
    print Sin(PI/2)

    print "Tan"
    print Tan(0)
    if Abs(Tan(PI)) < 0.00001 then
        print "close enough to 0"
    end if

    print "Acos, Asin, Atan, Atan2"
    print Acos(0.25)
    print Trunc(Acos(0.25) * 1000)
    print Asin(0.25)
    print Atan(0.25)
    print Atan2(1, 0.25)

    print "Log, Log10, Exp"
    print Round(Log(EULER) * 1000)
    print Log10(1000)
    print Floor(Exp(5))

    print "Floor, Ceil"
    print Floor(5.5)
    print Floor(-6.5)
    print Ceil(5.5)
    print Ceil(-6.5)

    print "Round"
    print Round(5.4)
    print Round(5.5)
    print Round(5.6)
    print Round(6.4)
    print Round(6.5)
    print Round(6.6)
    print Round(-5.4)
    print Round(-5.5)
    print Round(-5.6)
    print Round(-6.4)
    print Round(-6.5)
    print Round(-6.6)

    print "Trunc"
    print Trunc(5.4)
    print Trunc(5.5)
    print Trunc(5.6)
    print Trunc(6.4)
    print Trunc(6.5)
    print Trunc(6.6)
    print Trunc(-5.4)
    print Trunc(-5.5)
    print Trunc(-5.6)
    print Trunc(-6.4)
    print Trunc(-6.5)
    print Trunc(-6.6)
end sub
--output--
^
257
16
Abs
5
5
Sqr
3
NaN
Cos
1
-1
Sin
0
1
Tan
0
close enough to 0
Acos, Asin, Atan, Atan2
1.318116071652817966608495225955266
1318
0.252680255142078646901637739574653
0.2449786631268641434733268624768242
1.325817663668032553303532949939836
Log, Log10, Exp
1000
3
148
Floor, Ceil
5
-7
6
-6
Round
5
6
6
6
6
7
-5
-6
-6
-6
-6
-7
Trunc
5
5
5
6
6
6
-5
-5
-5
-6
-6
-6
