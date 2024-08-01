using System;

namespace MyNamespace
{
    public class MyClass
    {
        private int myPrivateField;
        public string MyProperty { get; set; }

        public MyClass()
        {
            myPrivateField = 0;
            MyProperty = "Hello, world!";
        }

        public void MyMethod()
        {
            Console.WriteLine(MyProperty);
        }

        public int MyMethodWithParameters(int a, int b)
        {
            return a + b;
        }

        public static void Main(string[] args)
        {
            MyClass myObject = new MyClass();
            myObject.MyMethod();

            int result = myObject.MyMethodWithParameters(5, 10);
            Console.WriteLine(result);
        }
    }
}