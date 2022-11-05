# Working With the LLVM AST and Using it to Make Changes to Clang-Tidy Source

There are situations where a particular clang-tidy check will report a lot of false positives or require some feature
additions based on the use case. Solving either problem will require making changes to clang-tidy source and require
both understanding the AST and working with the AST to make code changes as all the matchers in clang-tidy source are
reliant on the AST and written using the AST. The following sections go into resources and examples that will be helpful
in gaining a better understanding of the LLVM AST and writing code using it.

## Table of Contents
- [Understanding the AST](#understanding-the-ast)
- [How to Inspect/Verify New AST Matchers](#how-to-inspectverify-new-ast-matchers)
    - [Using Clang-Query on Source Code](#using-clang-query-on-source-code)
    - [Using clang-query on Compiler Explorer vs Using it in your
      Terminal](#using-clang-query-on-compiler-explorer-vs-using-it-in-your-terminal)
- [Debugging the AST](#debugging-the-ast)
- [Understanding How Registered Matchers Work](#understanding-how-registered-matchers-work)
- [Adding and Debugging Regression and Unit Tests for
  Clang-Tidy](#adding-and-debugging-regression-and-unit-tests-for-clang-tidy)
- [Capturing Failed Tests for Manual Debugging](#capturing-failed-tests-for-manual-debugging)

## Understanding the AST

The following list of resources goes into the basics and details of understanding the LLVM AST:

- [Understanding the Clang AST](https://jonasdevlieghere.com/understanding-the-clang-ast/) - Goes over LLVM AST basics,
  traversing the AST, important terms and examples. The link also contains a video on the AST and is useful to watch.

- [Official LLVM AST Tutorial](https://clang.llvm.org/docs/IntroductionToTheClangAST.html) - Official LLVM Tutorial on
  the AST. The tutorial contains links to important concepts like ASTContext, AST Nodes and other AST basics.

- [LLVM AST Classes Documentation](https://clang.llvm.org/doxygen/namespaceclang.html) - LLVM Doxygen Documentation for
  the AST. This serves as a reference for all the LLVM AST specific classes and methods available.

- [LLVM AST Matchers Reference](https://clang.llvm.org/docs/LibASTMatchersReference.html) - An important reference that
  documents all the available matchers, their syntax and the correct parameters that they accept.

- [Writing a Custom Clang-Tidy Check](http://blog.audio-tk.com/2018/03/20/writing-custom-checks-for-clang-tidy) - This
  resource is helpful for understanding the structure of a clang-tidy check and how to write custom checkers or use that
  knowledge to modify existing checkers.

- [P. Goldsborough “clang-useful: Building useful tools with LLVM and clang for fun and
  profit"](https://www.youtube.com/watch?v=E6i8jmiy8MY) - A good video tutorial from CppNow-2017 on the LLVM AST.

## How to Inspect/Verify New AST Matchers

Let's say you have just written an AST matcher to match something in your code, but want to verify quickly if the
matcher did indeed match what it's supposed to match: one way to do that is to build clang-tidy and debug the matcher
using a regression test to see whether the matcher returns the correct thing. This would work, but in order to do that
you'd need to build the source, run the debugger and inspect values, which can be time consuming if you want to verify
the match right away.

To verify AST matchers quickly, you can use clang-query. clang-query is a tool which one can run on a source file, write
various matchers on the command-line interface and see whether it matches the source that it should.

Microsoft has an excellent and detailed resource on using clang-query to inspect AST matchers - [Examining AST Matchers
with
clang-query](https://devblogs.microsoft.com/cppblog/exploring-clang-tooling-part-2-examining-the-clang-ast-with-clang-query/).

Here are some key points to keep in mind when trying to verify an AST matcher:

### Using Clang-Query on Source Code

clang-query is a very new tool added to the clang toolkit and is therefore limited in what it can do. For instance if
you want to test your matcher in RTC code, that'll be complicated since clang-query has issues with custom headers
included. At this point, clang-query is something that's useful for prototyping and quickly verifying the correctness of
an AST matcher. If you'd like to use it to test for a specific use case then it'd be better to write a small toy example
and run clang-query on it.

For example, if one wants to write a matcher that matches any calls made to ```cpp std::move()``` a toy example can be
written to make calls to `std::move()` and the matcher can be tested using clang-query.

As of now the only way to test your matcher on RTC code is to build clang-tidy with your changes and run your custom
binary using the clang-tidy script to observe whether it matches any violations or does not repeat any false positive
violations.


### Using clang-query on Compiler Explorer vs Using it in your Terminal

It is highly advisable to do all clang-query experimentation on [Compiler Explorer](www.godbolt.org). The clang-query
command line tool as of now has very poor formatting capability. For example, the following matcher matches any calls
made to `std::move()`:

```cpp
m callExpr(callee(functionDecl(hasName("std::move"))), argumentCountIs(1))
```

Now while typing the matcher out, perhaps you miss a closing `"` in the argument to `hasName("std::move")`. You might
immediately think to move your cursor and correct it, but unfortunately the command line tool does not have this
capability and you will have re-type the whole checker again. The compiler explorer clang-tidy extension allows you to
format such typos normally, making clang-query a lot easier to use.

clang-query is available on compiler explorer by clicking the `Add tools` from the compiler pane and clicking on
`clang-query`. A matcher can be typed into the `StdIn` panel by clicking on the `StdIn` option which opens a textbox for
entering the matcher. A sample configuration with a matcher that matches calls to `std::move()` can be viewed by
clicking on this link - [clang-query on compiler explorer](https://godbolt.org/z/aP7z6q).

clang-query might also match on any headers included. To view only the matches on your source file just look out for
matches that have `<source>:` prefixed to them. These are matches from the source code that you have written.

## Debugging the AST

Now you have written an AST Matcher or used the AST to perform some sort of task in a clang-tidy checker, but perhaps
the changes are still not doing what you expect. Debugging the AST is one of the ways to see what is going on.

Debugging is very simple using VS Code. Debugging an AST Matcher or a part of a clang-tidy checker can be done in
conjunction with setting a breakpoint on the line that you'd like to debug and running a regression test for that
checker.

For example, assume that [this matcher](clang-tools-extra/clang-tidy/performance/UnnecessaryValueParamCheck.cpp#L80) in
the `performance-unnecessary-value-param` check is not returning anything even when the source file has a `std::move()`
in it: how do we debug it? One way to do this is to set a breakpoint and create a `launch.json` file which will launch
the debugger in VS Code. A sample `launch.json` file looks like this:

```
{
"version": "0.2.0",
    "configurations": [
        {
            "name": "g++ - Build and debug active file",
            "type": "lldb",
            "request": "launch",
            "stopAtEntry": true,
            "cwd": "${path/to/directory}",
            "program": "${path/to/debugger}",
            "args": [
                "${path/to/llvm}/clang-tools-extra/test/clang-tidy/performance-unnecessary-value-param.cpp",
                "--checks=-*,performance-unnecessary-value-param",
                "-format-style=none",
                "--",
                "-stdlib=libc++",
                "-std=c++11"
                "-nostdinc++"
            ]
        }
    ]
}
```

The file has the following parameters in `configuration`:

- "name": Name of the service/action to perform.
- "type": The debugger type, in this case lldb.
- "request": The debugging mode that VS Code should follow.
- "stopAtEntry": Boolean value to signify whether to stop or not on the first line of code.
- "cwd": The path to the directory where the source code is stored.
- "program": The path to the debugger.
- "args": arguments to pass to the debugger i.e path to the checker's test file, name of the checks to run, what C++
   standard and standard library to use etc.

For more details on how to set up debuggers in VS Code, please visit [debugging in VS
Code](https://code.visualstudio.com/docs/editor/debugging).

## Understanding How Registered Matchers Work

A clang-tidy checker has a `registerMatchers()` method in which a matcher can be written to match a particular condition
in source, for example to check for function arguments that are expensive to copy. When a clang-tidy checker gets
invoked, the `registerMatchers()` method can be seen as an entry point to the checker where a matcher is executed and if
it finds a match then it calls the `check()` method to handle the rest of the heavy lifting for the checker. A match can
be registered in `registerMatchers()` method via the `Finder->addMatcher()` method. A lot of clang-tidy checkers have
only a single matcher registered, as most of the times the checker only needs to match one particular thing. What if a
checker needs two or more matchers to go over different scenarios for a checker? Are the matchers concurrently executed
and the result passed on to `check()`?

The answer is no. If your checker needs to register more than one matcher, `registerMatchers()` runs the checkers
synchronously, the process is:

1. Call a matcher, transfer results to `check()` and run the code in `check()`.
2. Once the first matcher finishes this cycle, the other matcher is called and transfer is passed to `check()` again.
3. This cycle keeps going on till all the registered matchers have finished executing sequentially.

Understanding this is very important when a checker is being debugged. Say someone adds a new matcher in
`registeredMatchers()` and wants to debug this. Without this knowledge they'll only see the first matcher and its
associated variables being filled up in `check()` and nothing being filled up for associated variables for the second
matcher. This might lead to the confusion that the matcher is either failing or not being called, but in fact after the
first matcher completes running in `check()`, the debugger will jump back to `registerMatchers()` to call the second
matcher.

## Adding and Debugging Regression and Unit Tests for Clang-Tidy

Most of clang-tidy's tests for checkers are written as regression tests in [Clang-Tidy Regression
Tests](clang-tools-extra/test/clang-tidy) and it is the recommended place to add tests for any new or existing checkers.
The test names do not follow a uniform pattern or guideline. For example tests for `performance-value-unnecessary-param`
follow a `positive<Name-of-the-test>(...)` pattern for tests where the checker will make changes, and
`negative<Name-of-the-test>(...)` for tests where the checker will not make any changes. On the other hand, tests for
`performance-move-const-arg` follow no such pattern and the tests are named `f<1-n>(...)` in increasing order. When
adding tests after making changes to an existing checker, it is recommended to follow the existing pattern in the test
file. When adding tests for any new checker, it is best to follow a test naming pattern that describes the intent of the
test to any reviewer or developer working on the checker easily.

Regression and unit tests for `clang-tidy` are run via [this Python
script](clang-tools-extra/test/clang-tidy/check_clang_tidy.py). The script uses a tool called `FileCheck` to make any
comparisons between the original test file and a temporary output file that is stored in the `build/` directory where
`clang-tidy` is built. The link where the output files is
`<build-dir-name>/tools/clang/tools/extra/test/clang-tidy/Output` where the output files are stored in the format
`<checker-name>.tmp.cpp.orig`. `FileCheck` compares these two files to see if there is a mismatch anywhere in the
output. If a mismatch is found, the script returns an error and a build failure when building clang-tools-extra along
with running unit tests and regression tests. Every tests where a change should occur has a `// CHECK-FIXES:` line with
the change, for example:


```cpp
void positiveExpensiveConstValue(const ExpensiveToCopyType Obj);
// CHECK-FIXES: void positiveExpensiveConstValue(const ExpensiveToCopyType& Obj);
```

The test, when run through the `performance-unnecessary-value-param` checker, changes the argument to be passed by a
constant reference, as specified in the check-fixes comment. `FileCheck` parses these comments in a test file to
construct what the final output should look like. If a mismatch occurs then the test errors out.

## Capturing Failed Tests for Manual Debugging

As detailed in the previous section a test file can error out if `FileCheck` detects a mismatch in the output file it
constructs by parsing `// CHECK-FIXES:` in the test file and the temporary output file that is created by running the
checker on a test file. When such errors occur, the error messages generated are not easy to decipher. `FileCheck` just
prints out all the output generated by the checker so far and the line where the error occurred. Using the debugger
seldom leads to a quick solution on what is wrong and for these situations the test requires manual debugging.

In such situations, it is always useful to first see the name of the test that is failing and what the test does. Then
check the temporary file to see what was the output produced by running that test through the checker. In some cases the
issue is apparent, since the mismatch is clearly due to the test being faulty and the issue can be rectified easily.

In other cases it can be seen that the checker did not do what it was supposed to i.e make the changes or make no
changes at all. In such cases it is useful to comment out all other tests except for the failing test and run the
debugger on it to see what the checker is doing to the the code in the test.
