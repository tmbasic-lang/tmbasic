#procedure
sub Main()
    const NUM_ROWS = 22
    dim row = [0]
    for rowNumber = 1 to NUM_ROWS
        dim list newRow
            yield 1
            for colIndex = 1 to Len(row) - 1
                yield row(colIndex) + row(colIndex - 1)
            next
            yield 0
        end dim

        row = newRow

        for i = 0 to NUM_ROWS - rowNumber
            print " ";
        next
        for i = 0 to Len(row) - 2
            if (row(i) mod 2) = 0 then
                print "  ";
            else
                print "█ ";
            end if
        next
        print ""
    next
end sub
--output--
                      █ 
                     █ █ 
                    █   █ 
                   █ █ █ █ 
                  █       █ 
                 █ █     █ █ 
                █   █   █   █ 
               █ █ █ █ █ █ █ █ 
              █               █ 
             █ █             █ █ 
            █   █           █   █ 
           █ █ █ █         █ █ █ █ 
          █       █       █       █ 
         █ █     █ █     █ █     █ █ 
        █   █   █   █   █   █   █   █ 
       █ █ █ █ █ █ █ █ █ █ █ █ █ █ █ █ 
      █                               █ 
     █ █                             █ █ 
    █   █                           █   █ 
   █ █ █ █                         █ █ █ █ 
  █       █                       █       █ 
 █ █     █ █                     █ █     █ █ 
