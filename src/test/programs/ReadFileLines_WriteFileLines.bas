#procedure
sub Main()
    dim path = "./tmbasic-temp"
    WriteFileText path, "A" + NEW_LINE + "B" + NEW_LINE
    dim lines = ReadFileLines(path)
    print Len(lines)

    WriteFileLines path, ["C", "D"]
    dim text = ReadFileText(path)
    if text = "C" + NEW_LINE + "D" + NEW_LINE then
        print "good"
    end if

    DeleteFile path
end sub
--output--
2
good
