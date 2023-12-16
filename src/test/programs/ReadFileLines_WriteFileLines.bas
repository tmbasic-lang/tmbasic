#procedure
sub Main()
    dim path = "./tmbasic-temp"
    WriteFileText path, "A" + NewLine() + "B" + NewLine()
    dim lines = ReadFileLines(path)
    print Len(lines)

    WriteFileLines path, ["C", "D"]
    dim text = ReadFileText(path)
    if text = "C" + NewLine() + "D" + NewLine() then
        print "good"
    end if

    DeleteFile path
end sub
--output--
2
good
