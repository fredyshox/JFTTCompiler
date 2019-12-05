#!/usr/bin/env python3
# Compiler executable tester
# Usage: 
# python3 autoTest.py <path_to_compiler_exe> <path_to_vm_exe>
# 
# Make sure that your cwd is project root dir!

import unittest
import subprocess
import sys
import tempfile
import os

cExePath = None
vmExePath = None
tempOutputPath = None

programs = [
    "resources/0-div-mod.imp",
    "resources/1-numbers.imp",
    #"resources/2-fib.imp",
    #"resources/3-fib-factorial.imp",
    "resources/4-factorial.imp",
    "resources/5-tab.imp",
    "resources/6-mod-mult.imp",
    "resources/7-loopiii.imp",
    "resources/8-for.imp",
    "resources/9-sort.imp"
]
inputs = [
    ["5", "2"],
    ["2"],
    #[],
    #[],
    ["10"],
    [],
    ["1234567890", "1234567890987654321", "987654321"],
    ["1", "2", "3"],
    ["12", "23", "34"],
    []

]
outputs = [
    ["1", "2", "0", "1"],
    ["0", "1", "2", "10", "100", "10000", "1234567890", "17", "15", "999", "555555555", "7777", "999", "11", "707", "7777"],
    #[],
    #[],
    ["3628800"],
    ["0", "24", "46", "66", "84", "100", "114", "126", "136", "144", "150", "154", "156", 
     "156", "154", "150", "144", "136", "126", "114", "100", "84", "66", "46", "24", "0"],
    ["674106858"],
    ["31001", "40902", "2222013"],
    ["507", "4379", "0"],
    ["5", "2", "10", "4", "20", "8", "17", "16", "11", "9", "22", "18", "21", "13", "19", "3", "15", "6", "7", "12", "14", "1",
     "1234567890", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22"]

]

class CompilerTest(unittest.TestCase):
    def handler(self, program, expectedOutput, userInput):
        global vmExePath, cExePath, tempOutputPath
        try:
            res = subprocess.run([cExePath, program, tempOutputPath], 
                                 stdout=subprocess.DEVNULL, 
                                 stderr=subprocess.DEVNULL, 
                                 timeout=5.0)
            self.assertEqual(res.returncode, 0)
        except:
            self.fail("Compiler subprocess creation failed")
        
        actualOutput = []
        with subprocess.Popen([vmExePath, tempOutputPath], stdout=subprocess.PIPE, stdin=subprocess.PIPE) as vmProc:
            try:
                inputStr = str.encode(os.linesep.join(userInput) + os.linesep) if len(userInput) != 0 else None
                outs, _ = vmProc.communicate(input=inputStr, timeout=5.0)
                outs = outs.decode()
                for line in outs.split(os.linesep):
                    index = line.rfind("> ")
                    if index != -1 :
                        value = line[index + 2:].rstrip()
                        actualOutput.append(value)
            except subprocess.TimeoutExpired:
                self.fail("Timeout expired.")

        self.assertEqual(len(actualOutput), len(expectedOutput))
        for a, e in zip(actualOutput, expectedOutput):
            self.assertEqual(a, e)
        print("STATUS: Completed test for {}".format(program))

    def test_output(self):
        for i in range(len(programs)):
            with self.subTest(program=programs[i]):
                path = os.path.abspath(programs[i])
                self.handler(path, outputs[i], inputs[i]) 

if __name__=="__main__":
    if len(sys.argv) < 3:
        print("Usage: {} <path_to_compiler_exe> <path_to_vm_exe>".format(sys.argv[0]))
        exit(1)

    cExePath = os.path.abspath(sys.argv[1])
    vmExePath = os.path.abspath(sys.argv[2])
    tempOutputPath = tempfile.mkstemp(suffix='.mr')[1]
    print(tempOutputPath)
    unittest.main(argv=[""], exit=False)
    os.remove(tempOutputPath)

