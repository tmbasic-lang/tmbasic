#procedure
sub Main()
    dim c = NewLabel()
    SetControlBounds c, { Left: 1, Top: 2, Width: 3, Height: 4 } as Rectangle
    dim r2 = ControlBounds(c)
    print r2.Left
    print r2.Top
    print r2.Width
    print r2.Height
end sub
--output--
1
2
3
4
