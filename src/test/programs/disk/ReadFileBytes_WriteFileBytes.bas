sub Main()
    WriteFileBytes "myfile.dat", [65, 66, 67]
    print ReadFileText("myfile.dat")

    WriteFileText "myfile.dat", "ABC"
    dim x = ReadFileBytes("myfile.dat")
    print Len(x)
    print x(0)
    print x(1)
    print x(2)

    DeleteFile "myfile.dat"
end sub
--output--
ABC
3
65
66
67
