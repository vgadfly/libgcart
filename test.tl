// test object

// builtin types
one#deadbeef i: int s: string l: long d: double b: Bool = One;

// objects
two#1337f001 t1: One t2: One = Two;

// vector
three#c001babe vo: Vector<Two> vi: Vector<int> vs: Vector<string> = Three;

// flags
test.four#55aa33cc flags: # i: flags.0?int s: flags.1?Vector<bytes> = test.Four;
