# Accountant-C
A simple accountant database softare, that I wrote to keep track of my expenses in highschool in 2008. This is a study project not used for commercial purpouse. 



## CONFIGURE ACCOUNTANT

#### Main Settings

The following describes each setting that could be set in acc.conf (the name is constant and thus no other names are accepted to use for configuration file) to use with Accountant. This file should be in the same directory in which the program is. It consists of setting name equal sign and a value - all these on the same line. 

First its written the name of the setting (it should be typed as is shown if you want the program to read the value) and then is given the value range ([x-y] from x to y; [a b c ...] a or b or c) as well as the default value that will be set if the specified value in acc.conf file is out of range or the setting is not present. The order in which you type the settings doesn't matter.

#app:date_separator_char  is just a comment and is not considered setting because it starts with #

app:date_separator_char = [/ , \ : -], default: '-'

	Used to separate the numbers of a date. Example formats are: 21-5-2007, 21.5.2007,
	21\5\2007, 21/5/2007, 21:5:2007. Only the chars showed above are allowed. The char may be
	quoted: "/".
	
	
app:use_iso_date_format = [1 (true) or 0 (false)], default: 1

	Determines the order in which date digits are shown. ISO format: Day-Month-Year. 
	Non ISO (US) format: Year-Month-Day.
	
	
app:value_precision = [0-6], default: 2

	Used in list to show digits with the precision given as value. Also digits entered in
	value field will be trimmed to the precision of the setting.
	
	
app:set_new_IDs_on_appload = [1 (true) or 0 (false)], default: 0

	When the program starts it loads all the records from the database in the RAM. So its
	possible to give the records new ID numbers corresponding to their number in the file. For 
	example record five will have ID = 5. But on the other hand it will be difficult for 
	you to remember new IDs each time and thus you may prefer to turn off this option.
	
	
app:last_database_used = [valid file name], default: 'main.adb'

	This setting is used extensively by the program. Each time you open or create new DB
	its name is stored in the value of the setting. When the program loads the DB pointed by the 		
	name is loaded. You can change this manually but have to ensure you are giving valid 
	file names. Quotes around the name of the file are allowed.
	
	
app:last_filter_expression = [valid filter expression], default: not set

	This setting is also changed by the program and you can edit it manually. Its used to 
	to filter all the data in some meaningful way. For more info check the file 
	"FILTER EXPRESSIONS_v0.1.TXT"
	
	
showlist:lines_per_page = [1-200], default: 23

	The number of lines(records) displayed per page when show list and show 
	filtered list invoked. Use 0(zero) to disable the function.
	
	
showlist:remember_last_shown = [1 (true) or 0 (false)], default: 0

	List data preview is very powerful. It remembers the last page you have viewed and the
	next time you open for example Show list > Income records it will show you the page you
	last were looking at. This setting controls weather the program to remember that page and
	the next time you start it and open the list, record preview will begin from the saved page.
	This option requires some to save the whole database even if there are no changes made to 
	the DB resulting in slowdown in databases with more than 5 000 000 records.
	
	
export:max_width = [45-300], default: 60
	
	Whit this setting you can limit the maximum number of chars that are written to the output
	(exported) file. Its created for printers compatibility.
	
	
export:add_typestats = [1 (true) or 0 (false)], default: 1

	Adds type usage to the exported file - how much items a type have and the total sum of all of them.
	
	
export:both_path = [valid file name], default: 'export_all.txt'
export:income_path = [valid file name], default: 'export_income.txt'
export:outcome_path = [valid file name], default: 'export_outcome.txt'

	These three settings are used to set paths in which export data will be saved. 
	They are used by the Export to txt part of the program. The names may be quoted: 
	"file name".
	
#### Grouping

From version 0.3.x you can group similar settings at one place. There are several categories available now: app, export and showlist. Settings that belong to one of these groups can be written like that:

	
	#use <app> to start a group
	<app>
		value_precision = 3
		last_database_used = <some_filename_you choose>
	</app> 
	#the backslash means that you close that group thus the setting below is invalid
	use_iso_date_format = 0
	
	#but if the setting is written that way it is valid: 
	app:use_iso_date_format = 0
	
	#the same rules are used for the setting group below
	<export>
		add_typestats = 1
		max_width = 50
	</export>
	
Inside groups settings can start with white space(s) or tabs and will be read correctly. 


## FILTER EXPRESSIONS
					    
1. General Info 					    
2. Priority Rules 
3. The #SORT expression
4. Examples

	
#### 1. General Info 
		
Filter expressions help you display DB information for current date or according to some rules you assign. For example if you have more than 1000 records in your database and you want to find which incomes are, say, added on the date 24.5.2006 or are greater than 253.33 units( $, euros ...) the only thing you can do is to use filter expression. For the example above the expression is:
	
	Example 1. 
	'#DATE = 24.5.2006  #VALUE > 253.33'.
	
However there are some restrictions and priority rules that must be mentioned.
	
There are five parameter types available to set in a filter expression: #ID, #DATE, #VALUE, #TYPE and #SORT. These can also be combined together to form more complex statements (up to 30 parameters). The parameter type (e.g. #ID, #TYPE ...) and the parameter value (different for each param. type; see below) can be separated by equality signs such as > < = , all forming six combinations:
	
	<  - less then
	>  - greater then
	=  - equal to
	<= - less then or equal to
	>= - greater then or equal to
	<> - not equal
	
	Only for strings (gives error if used with filter type different from #TYPE):
	~  - contains
	!~ - not contains
	
Here is the the table describing what values can be passed to the parameter value:

	#ID <signs> [non negative number from 0 to 4,294,967,295 representing the record's index] 
	
	#DATE <signs> [valid date in the formats Day-Month-Year;  Year-Month-Day]
	
	#VALUE <signs> [non negative number representing valid currency unit]
	
	#TYPE <signs> "[string not longer than 2000 chars]"
	
	#SORT <signs> "[string not longer than 2000 chars in the format <sort_by_criteria>:<sort_direction>, ID:ASC]"
	
	You can leave as much white spaces between the parameter type, the <signs> and the parameter value as you wish: 
	
	Example 2.
	'    #ID   <23654#VALUE <> 200   #TYPE =  "Bank deposit" #ID>20'. 
	
The #TYPE value is the string inside the quotes (the quote signs are not considered value). In the above example there is one more tricky part - the two #ID parameters. Do they conflict each other? Read below.
	
	
#### 2. Priority Rules

It's very important to know exactly what are the priority rules in one filter expression. While being parsed each parameter of the filter is checked for conflicts with the already added parameters. If there is no conflict (as in Example 2.for #ID because 23654 > 20) the param. pair parsed is added to the list with parameters. But in the following preview:
	
	Example 3.
	'#ID >= 1000 #VALUE < 200 #ID>20 #VALUE > 200'
	
happens the following:
	1. Check for conflicts. Add Param. (ID >= 1000)
	2. Check for conflicts. Add Param. (VALUE < 200)
	3. Check for conflicts (ID>20).
	4. Conflict found and old Param. (ID >= 1000) is disabled. Add Param. (ID > 20).
	5. Check for conflicts (VALUE > 200).
	6. Conflict found and old Param. (VALUE < 200) is disabled. Add Param. (VALUE > 200).
	
and finally the filter parameters left are: '#ID>20 #VALUE > 200'. Parameters with = (equal) sign disable any previous parameters of the same type. All valid (not disabled) parameters are applied to every single record in the order they have been added. Thus an AND relation occurs between the elements of the filter (param.1 AND param.2 AND pram.n ...).


#### 3. The #SORT expression

The #SORT expr. supports ID, DATE, VALUE and TYPE. There are two possible directions ASC and DESC. So if you specify VALUE:DESC the value filed will be sorted decsending (from the biggest tho the smallest value). 


#### 4. Examples

Here are some more examples:
	
	'#DATE >= 1/1/2006  #DATE < 1/1/2007  #VALUE > 10  #TYPE <> "Bank deposit"'.
This example shows all records between the dates written with value bigger than 10 and that are different from 'Bank deposit'.
	
	'#ID >= 10  #TYPE ~ "tic"  #ID <= 1000  #TYPE ~ "tac" #SORT = DATE:ASC'
This filter shows all records from 10 to 1000 included and all that contain both "tic" and "tac" no matter weather the strings are in the #TYPE. If the record for example contains only "tic" it would not be shown.
  

## Switches

- Accountant -t <filename> 	= traverses the DB specified by flename
- Accountant -t = traverces the last used DB
- Accountant <filename> = opens the DB specified by filename for usage but does not save it as app:last_db_used


