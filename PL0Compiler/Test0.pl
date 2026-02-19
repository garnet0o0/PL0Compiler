const pi = -3.1415926;
var n, a;
/*This is a comment*/
//comment line
begin
  n := 1;
  a := +5.2e5;
  if (n=1) write(pi);
  else write(a);
  while n>=-1 do begin n:=n-1; a:=a/10; write(n); write(a); end
  
  do n:=n+1; while n<=1;
end.
