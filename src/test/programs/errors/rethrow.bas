sub Main()
    try
        try
            throw 1, "aaa"
        catch
            rethrow
        end try
    catch
        print ErrorMessage()
    end try
end sub
--output--
aaa
