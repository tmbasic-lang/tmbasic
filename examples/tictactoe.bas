' Board(r)(c) is "X", "O", or ""
dim shared Board as List of List of String

sub Main()
    Board = [["X", "O", ""], ["", "", ""], ["", "", ""]]

    EnterFullscreen
    for frame = 1 to 4000
        SetBackColor 32, 41, 3
        Cls
        DrawTitlebar
        SetBackColor 32, 41, 3
        SetForeColor 255, 255, 255
        DrawBoard
        NextFrame
    next
end sub

sub DrawTitlebar()
    SetBackColor 255, 255, 255
    SetForeColor 0, 0, 0
    for i = 0 to ScreenWidth()
        MoveCursor i, 0
        print " "
    next
    MoveCursor 1, 0
    print "Tic Tac Toe"
end sub

sub DrawX(x as Number, y as Number, size as Number)
    SetForeColor 255, 154, 149
    
    dim margin = size / 6
    dim centerX = x + size / 2
    dim centerY = y + size / 2
    dim radius = (size / 2) - margin
    
    ' Generate points for one eighth of the X (45 degree segment)
    dim steps = Floor(radius * 1.5)  ' More steps for better connectivity
    for i = 0 to steps
        dim ratio = i / steps
        dim offset = ratio * radius
        dim px = Floor(centerX + offset + 0.5)
        dim py = Floor(centerY + offset + 0.5)

        ' Draw X at each point
        MoveCursor px, py  ' Bottom-right
        print "X"
        MoveCursor 2 * centerX - px, 2 * centerY - py  ' Top-left
        print "X"
        MoveCursor 2 * centerX - px, py  ' Bottom-left
        print "X"
        MoveCursor px, 2 * centerY - py  ' Top-right
        print "X"
        
        ' Mirror across 45 degrees for better coverage
        dim dx = px - centerX
        dim dy = py - centerY
        dim px2 = centerX + dy
        dim py2 = centerY + dx
        
        if px2 <> px or py2 <> py then
            MoveCursor px2, py2
            print "X"
            MoveCursor 2 * centerX - px2, 2 * centerY - py2
            print "X"
            MoveCursor 2 * centerX - px2, py2
            print "X"
            MoveCursor px2, 2 * centerY - py2
            print "X"
        end if
    next
end sub

sub DrawO(x as Number, y as Number, size as Number)
    SetForeColor 133, 157, 213
    
    dim radius = size / 3
    dim centerX = x + size / 2
    dim centerY = y + size / 2
    
    ' First, store the points of the first quadrant (0-90 degrees)
    dim list quadPoints
        for angle = 0 to 90 step 5
            dim radians = angle * 3.14159 / 180
            dim pointX = centerX + cos(radians) * radius
            dim pointY = centerY - sin(radians) * radius
            yield [Floor(pointX + 0.5), Floor(pointY + 0.5)]
        next
    end dim
    
    ' Draw all four quadrants by mirroring the points
    for each point in quadPoints
        dim px = point(0)
        dim py = point(1)
        
        ' Draw in all four quadrants
        ' Top-right quadrant (original)
        MoveCursor px, py
        print "O"
        
        ' Top-left quadrant (mirror X)
        MoveCursor 2 * centerX - px, py
        print "O"
        
        ' Bottom-right quadrant (mirror Y)
        MoveCursor px, 2 * centerY - py
        print "O"
        
        ' Bottom-left quadrant (mirror both X and Y)
        MoveCursor 2 * centerX - px, 2 * centerY - py
        print "O"
    next
end sub

sub DrawBoard()
    dim w = ScreenWidth()
    dim h = ScreenHeight()
    
    dim size = w
    if h < w then size = h
    
    ' Calculate the horizontal offset to center the board
    dim xOffset = Floor((w - size * 1.5) / 2)
    
    ' x1 x2
    '  |  |
    '--+--+-- y1
    '  |  |
    '--|--|-- y2
    '  |  |
    
    dim cellWidth = Floor(size * 1.5 / 3)
    dim cellHeight = Floor(size / 3)
    
    dim x1 = xOffset + cellWidth
    dim x2 = xOffset + 2 * cellWidth
    dim y1 = cellHeight
    dim y2 = 2 * cellHeight
    
    ' Vertical lines
    for y = 2 to size - 3
        MoveCursor x1, y
        print "|"
        MoveCursor x2, y
        print "|"
    next
    
    ' Horizontal lines
    for x = xOffset + 2 to xOffset + size * 1.5 - 3
        MoveCursor x, y1
        print "-"
        MoveCursor x, y2
        print "-"
    next
    
    ' Crosses
    MoveCursor x1, y1
    print "+"
    MoveCursor x2, y1
    print "+"
    MoveCursor x1, y2
    print "+"
    MoveCursor x2, y2
    print "+"
    
    ' Xs and Os - center them within the wider cells
    for r = 0 to 2
        for c = 0 to 2
            dim xo = Board(r)(c)
            if xo = "X" or xo = "O" then
                ' Calculate the center position for the symbol
                dim symbolSize = cellHeight  ' Keep symbols square
                dim cellX = xOffset + c * cellWidth
                dim cellY = r * cellHeight
                ' Center the symbol horizontally in the wider cell
                dim symbolX = Floor(cellX + (cellWidth - symbolSize) / 2)
                
                if xo = "X" then
                    DrawX symbolX, cellY, symbolSize
                else
                    DrawO symbolX, cellY, symbolSize
                end if
            end if
        next
    next
end sub

