#include <string>
#include <sstream>
#include <vector>
#include "Kernel.h"
#pragma once

class Shell
{
public:
	Shell(){}
	void CommanLineInterface()
	{
		std::string command;
		char* dir = new char[256];
		do
		{
			mykenrel.Set_Register0(14);
			mykenrel.Set_Register1(dir);
			mykenrel.System_Call();
			output("\n");
			output(dir);
			output(">");
			command = CommandInput();
			std::string result = transducer(command);
			output(result);
		} while (command != "quit");
	}
private:
	kernel mykenrel;
	std::string CommandInput()
	{
		std::string result;
		char temp='\0';
		do
		{
			mykenrel.Set_Register0(1);
			mykenrel.Set_Register2(temp);
			mykenrel.System_Call();
			temp = (char)mykenrel.Get_Register1();
			if(temp!='\n')result += temp;
		} while (temp != '\n');
		return result;
	}

	std::string transducer(std::string command)
	{
		std::string result;
		try
		{
			std::vector<std::string>token = makeToken(command);
			if (token[0]=="dir")
			{
				result += getDir();
			}
			else if (token[0] == "cd")
			{
				if (token.size() == 2)
				{
					result += changeDir(token[1]);
				}
				else throw(99);
			}
			else if (token[0] == "copy")
			{
				if (token.size() == 3)
				{
					result += copyFile(token[1], token[2]);
				}
				else throw(99);
			}
			else if (token[0] == "delete")
			{
				if (token.size() == 2){ result += deleteFile(token[1]); }
				else throw(99);
			}
			else if (token[0] == "deleteD")
			{
				if (token.size() == 2){ result += deleteDirectory(token[1]); }
				else throw(99);
			}
			else if (token[0] == "mkdir")
			{
				if (token.size() == 2){ result += makeDir(token[1]); }
				else throw(99);
			}
			else if (token[0] == "dstat")
			{
				result += systemInfo();
			}
			else if (token[0] == "dfile")
			{
				if (token.size() == 2){ result += fileContents(token[1]); }
				else throw(99);
			}
			else if (token[0] == "quit")
			{
				result += quit();
			}
			else
			{
				throw (100);
			}
		}
		catch (int e)
		{
			switch (e)
			{
			case(99):
				result+=("Invalid Parameters\n");
				break;
			case(100) :
				result += ("Invalid Command\n");
				result += "Commands are: dir\ncd (name)\ncopy (fileName);(Destination)\ndelete (FileName)\nmkdir (Name)\ndeleteD (name)\ndstat\ndfile (name)\nquit\n";
				break;
			default:
				result += ("System Error\n");
				break;
			}
		}
		return result;
	}
	std::string changeDir(std::string name)
	{
		mykenrel.Set_Register0(4);
		mykenrel.Set_Register1(name.c_str());
		mykenrel.System_Call();
		if (mykenrel.Get_Register3() != 0)
		{
			throw mykenrel.Get_Register3();
		}
		if (name == ".")
		{
			return "Stayed in this directory";
		}
		if (name == "..")
		{
			return "Moved up a directory";
		}
		std::string result = "Moved to " + name + " directory";
		return result;
	}
	std::string getDir()
	{
		mykenrel.Set_Register0(13);
		mykenrel.System_Call();
		if (mykenrel.Get_Register3() != 0)
		{
			throw mykenrel.Get_Register3();
		}
		return intToString(mykenrel.Get_Register1());
	}

	std::string deleteDirectory(std::string name)
	{
		mykenrel.Set_Register0(5);
		mykenrel.Set_Register1(name.c_str());
		mykenrel.System_Call();
		int e = mykenrel.Get_Register3();
		if (e != 0)throw(e);
		return name + " was deleted";
	}

	std::string copyFile(std::string fileName, std::string destination)
	{
		//open file
		mykenrel.Set_Register0(7);
		mykenrel.Set_Register1(fileName.c_str());
		mykenrel.System_Call();
		int e = mykenrel.Get_Register3();
		if (e != 0)throw (e);
		FILE* myfile = (FILE*)mykenrel.Get_Register1();
		//read file contents
		std::string fileContent="";
		while (!feof(myfile))
		{
			mykenrel.Set_Register0(9);
			mykenrel.Set_Register1(myfile);
			mykenrel.System_Call();
			fileContent+=mykenrel.Get_Register2();
		}
		//close file
		mykenrel.Set_Register0(11);
		mykenrel.Set_Register1(myfile);
		mykenrel.System_Call();
		e = mykenrel.Get_Register3();
		if (e != 0)throw(e);
		
		//change dir
		changeDir(destination.c_str());

		//create file and write file
		mykenrel.Set_Register0(8);
		mykenrel.Set_Register1(fileName.c_str());
		mykenrel.System_Call();
		e = mykenrel.Get_Register3();
		if (e != 0)throw(e);
		myfile = (FILE*)mykenrel.Get_Register1();
		for (int i = 0; i < fileContent.length();i++)
		{
			mykenrel.Set_Register0(10);
			mykenrel.Set_Register1(myfile);
			mykenrel.Set_Register2(fileContent[i]);
			mykenrel.System_Call();
			e = mykenrel.Get_Register3();
			if (e != 0)throw(e);
		}

		//close file
		mykenrel.Set_Register0(11);
		mykenrel.Set_Register1(myfile);
		mykenrel.System_Call();
		e = mykenrel.Get_Register3();
		if (e!= 0)throw(e);

		return "File Copied";
	}
	std::string quit()
	{
		mykenrel.Set_Register0(0);
		mykenrel.System_Call();
		if (mykenrel.Get_Register1() != 0)
			throw mykenrel.Get_Register1();
		return "Exited program";
	}
	std::string fileContents(std::string fileName)
	{
		mykenrel.Set_Register0(7); //open file to read content
		mykenrel.Set_Register1(fileName.c_str());
		mykenrel.System_Call();
		std::string filecontent;
		if (mykenrel.Get_Register3() != 0)
		{
			throw mykenrel.Get_Register3();
		}
				
		FILE* file=(FILE*)mykenrel.Get_Register1();
		while (!feof(file))
			{								//read file contents
				mykenrel.Set_Register0(9);
				mykenrel.Set_Register1(file);
				mykenrel.System_Call();
				if (mykenrel.Get_Register3() != 0)
				{
					throw mykenrel.Get_Register3();
				}
					
					
				filecontent += (char)mykenrel.Get_Register2();
			} 

			mykenrel.Set_Register0(11);
			mykenrel.Set_Register1(file);
			
			mykenrel.System_Call();
			if (mykenrel.Get_Register3() != 0)
			{
				throw mykenrel.Get_Register3();
			}

			return filecontent;
	}
	std::string systemInfo()
	{
		mykenrel.Set_Register0(15); //Set Register 0 = 15
		mykenrel.System_Call(); //call SystemCall() function
		if (mykenrel.Get_Register3() != 0) //If Register3 is not 0
		{
			throw mykenrel.Get_Register3(); //throw value in Register 3
		}
		std::string result = "Date ";
		result += (char*)mykenrel.Get_Register1();
		result += "; time ";
		result += (char*)mykenrel.Get_Register2();
		return result; //return contents of register 1 & 2 as strings
		
	}
	std::string makeDir(std::string dirName)
	{
		std::string results;
		mykenrel.Set_Register0(3);
		mykenrel.Set_Register1(dirName.c_str());
		mykenrel.System_Call();
		if (mykenrel.Get_Register3() != 0)throw(101);
		results = dirName + " Created";
		return results;
	}
	std::string deleteFile(std::string fileName)
	{
		mykenrel.Set_Register0(12);
		mykenrel.Set_Register1(fileName.c_str());
		mykenrel.System_Call();
		if(mykenrel.Get_Register3() != 0)
			throw mykenrel.Get_Register3();
		return "File deleted";

	}
	void output(std::string out)
	{
		for (int i = 0; i < out.length(); i++)
		{
			mykenrel.Set_Register0(2);
			mykenrel.Set_Register1(out[i]);
			mykenrel.System_Call();
		}
	}
	std::vector<std::string> makeToken(std::string com)
	{
		std::vector<std::string> vs;
		std::string s;
		std::istringstream is(com);
		while (!is.eof())
		{
			if (vs.size() == 0)
			{
				getline(is, s, ' ');
			}
			else
			{
				getline(is, s, ';');
			}
			vs.push_back(s);
		}
		return vs;
	}


	
	//Auxiliary
	std::string intToString(int x)
	{
		std::string result;
		char temp[256];
		//itoa(x,temp,10);
		//_itoa(x,temp,10);
		_itoa_s(x, temp, 255, 10);
		result = temp;
		return result;
	}

};