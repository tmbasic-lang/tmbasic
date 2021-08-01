#procedure
sub Main()
    ' deleting a file that already doesn't exist is not an error
    DeleteFile "this_file_does_not_exist"
    DeleteFile "this_file_does_not_exist"

    ' round trip a file
    DeleteFile "tmbasic-test"
    WriteFileText "tmbasic-test", "Hello, World!"
    print ReadFileText("tmbasic-test")
    DeleteFile "tmbasic-test"

    ' ReadFileText should throw an error if the file doesn't exist
    try
        print ReadFileText("this_file_does_not_exist")
    catch
        if ErrorCode = ERR_FILE_NOT_FOUND then
            print "1 ERR_FILE_NOT_FOUND"
        else
            print "1 "; ErrorCode; " "; ErrorMessage
        end if
    end try

    ' ReadFileText should throw an error if the directory doesn't exist
    try
        print ReadFileText("/this_directory_does_not_exist/this_file_does_not_exist")
    catch
        if ErrorCode = ERR_FILE_NOT_FOUND then
            print "2 ERR_FILE_NOT_FOUND"
        else
            print "2 "; ErrorCode; " "; ErrorMessage
        end if
    end try

    ' ReadFileText should throw an error if the filename is too long
    try
        print ReadFileText("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa")
    catch
        if ErrorCode = ERR_PATH_TOO_LONG then
            print "3 ERR_PATH_TOO_LONG"
        else
            print "3 "; ErrorCode; " "; ErrorMessage
        end if
    end try    

    ' WriteFileText should throw an error if the directory doesn't exist
    try
        WriteFileText "/missing/dir/tmbasic-test", "asdf"
    catch
        if ErrorCode = ERR_FILE_NOT_FOUND then
            print "4 ERR_FILE_NOT_FOUND"
        else
            print "4 "; ErrorCode; " "; ErrorMessage
        end if
    end try

    ' WriteFileText should throw an error if the filename is too long
    try
        WriteFileText "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", "asdf"
    catch
        if ErrorCode = ERR_PATH_TOO_LONG then
            print "5 ERR_PATH_TOO_LONG"
        else
            print "5 "; ErrorCode; " "; ErrorMessage
        end if
    end try

    ' WriteFileText should throw an error if the filename is actually a directory
    try
        WriteFileText ".", "asdf"
    catch
        if ErrorCode = ERR_PATH_IS_DIRECTORY then
            print "6 ERR_PATH_IS_DIRECTORY"
        else
            print "6 "; ErrorCode; " "; ErrorMessage
        end if
    end try

    ' DeleteFile should throw an error if the filename is too long
    try
        DeleteFile "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
    catch
        if ErrorCode = ERR_PATH_TOO_LONG then
            print "7 ERR_PATH_TOO_LONG"
        else
            print "7 "; ErrorCode; " "; ErrorMessage
        end if
    end try       
end sub
--output--
Hello, World!
1 ERR_FILE_NOT_FOUND
2 ERR_FILE_NOT_FOUND
3 ERR_PATH_TOO_LONG
4 ERR_FILE_NOT_FOUND
5 ERR_PATH_TOO_LONG
6 ERR_PATH_IS_DIRECTORY
7 ERR_PATH_TOO_LONG
