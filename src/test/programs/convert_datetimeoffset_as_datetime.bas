#procedure
sub Main()
    dim zone = TimeZoneFromName("America/Chicago")
    dim a = DateTimeOffsetFromParts(1776, 7, 4, 5, 15, 30, 450, zone)
    dim b = a as DateTime
    print b
end sub
--output--
1776-07-04 05:15:30.450
