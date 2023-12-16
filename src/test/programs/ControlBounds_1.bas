#procedure
sub Main()
    dim c = NewLabel()
    SetControlBounds c, 1, 2, 3, 4
    dim r = ControlBounds(c)
    print r.Left
    print r.Top
    print r.Width
    print r.Height
end sub
--output--
1
2
3
4
