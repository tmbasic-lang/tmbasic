sub Main()
    dim foo as Map from String to String
    foo("abc") = "def"
    print foo("abc")
end sub
--output--
def
