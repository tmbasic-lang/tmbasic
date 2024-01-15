sub Main()
    dim x = Rgb($1, $2, $3)
end sub

--output--
Error
50
$4 must be between 0 and 255.
--cases--
1|-1|0|0|Red
1|0|-1|0|Green
1|0|0|-1|Blue
1|256|0|0|Red
1|0|256|0|Green
1|0|0|256|Blue
