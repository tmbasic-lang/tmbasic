sub Main()
    dim ps = PathSeparator()
    dim path = "C:" + ps + "test" + ps + "file.txt"
    
    dim actualDirName = PathDirectoryName(path)
    if actualDirName = "C:" + ps + "test" then
        print "1 good"
    else
        print "1 "; actualDirName
    end if

    print "2 "; PathFileName(path)

    print "3 "; PathFileNameWithoutExtension(path)

    print "4 "; PathExtension(path)

    print "5 "; PathFileNameWithoutExtension("test")

    print "6 "; PathExtension("test")

end sub
--output--
1 good
2 file.txt
3 file
4 .txt
5 test
6 
