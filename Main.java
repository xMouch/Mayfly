
/*
Online Java - IDE, Code Editor, Compiler

Online Java is a quick and easy tool that helps you to build, compile, test your programs online.
*/




public class Main
{
    public static long fib(long n)
    {
        if(n <= 1)
            return n;
        return fib(n-1) + fib(n-2);
    }
    
    public static void main(String[] args) {
        for(long i = 0; i < 35; i = i+1)
	    {
            System.out.println(fib(i));
	    }
    }
}
