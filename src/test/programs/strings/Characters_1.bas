sub Main()
    ' U+0301 Combining Acute Accent
    dim s = "n" + "e" + Chr(769)
    print "String length: "; Len(s)
    dim x = Characters(s)
    print "Clusters length: "; Len(x)
    print "Cluster 0 length: "; Len(x(0))  ' x(0) is one code unit: "n"
    print "Cluster 1 length: "; Len(x(1))  ' x(1) is three code units: "e" + 0xCC 0x81
end sub
--output--
String length: 4
Clusters length: 2
Cluster 0 length: 1
Cluster 1 length: 3
