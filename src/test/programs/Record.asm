# create this structure:
# { v0: 123, o0: "hi", o1: { v0: 456 } }
RecordBuilderBegin 1 2 # outer record
LoadConstantA 123
RecordBuilderStoreA 0
LoadConstantStringX "hi"
RecordBuilderStoreX 0
RecordBuilderBegin 1 0 # inner record
LoadConstantA 456
RecordBuilderStoreA 0
RecordBuilderEnd
RecordBuilderStoreX 1
RecordBuilderEnd
PushX
RecordLoadA 0
NumberToString
StringPrint
LoadX 0
RecordLoadX 0
StringPrint
LoadX 0
RecordLoadX 1
RecordLoadA 0
NumberToString
StringPrint
Exit
