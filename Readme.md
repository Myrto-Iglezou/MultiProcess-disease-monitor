# System-Programming-project2

__Compile:__ ```make``` <br>
__Execution:__ ```./diseaseAggregator -w 5 -b 256 -i inputDir```<br>

## Goal of this project

The purpose of this paper is to familiarize students with creating processes using system calls fork / exec, communicating processes through pipes, using low-level I / O and creating bash scripts.

## Summary of the project

In this project, I hava implemented an undistributed information processing tool that receives, processes, records and answers questions about viruses.
Specifically, I implemented the diseaseAggregator application which creates a series of Worker processes that, together with the application, answer user questions.

## Project Description

### A. __The diseaseAggregator application (75%)__


The diseaseAggregator application will be used as follows: <br>


```./diseaseAggregator –w numWorkers -b bufferSize -i input_dir```


<br>where: <br>
- The **numWorkers** parameter is the Worker number of processes that the application will create. <br>
- The **bufferSize** parameter: is the size of the buffer for reading over pipes. <br>
- The **input_dir** parameter: is a directory that contains subdirectories with the files that will be processed by Workers. Each subdirectory will have the name of a country and will contain files named by dates, DD-MM-YYYY . For example *input_dir* could contain subdirectories China */ Italy / and Germany /* which have the following files: <br>

```/input_dir/China/21-12-2019``` <br>
```/input_dir/China/22-12-2019``` <br>
``` ...``` <br>
```/input_dir/Italy/31-01-2020``` <br>
```/input_dir/Italy/01-02-2020``` <br>
``` ... ``` <br>
```/input_dir/Germany/02-03-2020``` <br>

- Each DD-MM-YYYY file contains a series of patient records where each line describes a patient who was admitted / discharged to / from a hospital that day and contains the recordID, name, virus, and age. For example if the contents of the file / input_dir / Germany / 02-03-2020 of the file are:

```889 ENTER Mary Smith COVID-2019 23``` <br>
```776 ENTER Larry Jones SARS-1 87``` <br>
```125 EXIT Jon Dupont H1N1 62``` <br>


<p> When the program *diseaseAggregator* begins, numWorkers Workers child processes should start and distribute the subdirectories evenly with the countries in input_dir to Workers.  The program should start the Workers and inform each one of them via named pipe about the subdirectories that the Worker will undertake.</p>

<p> When the application (parent process) finishes creating the Worker processes, it will wait for input (commands) from the user from the keyboard (see commands below).</p>

<p> Each Worker process, for each directory assigned to it, will read all its files in chronological order based on the file names and fill in a series of data structures that it will use to answer questions prompted by the parent process.  If while reading files, the Worker detects a problematic record (eg in syntax or an EXIT record that does not have a previous ENTER record, etc.) then it will print an ERROR on a new line. When it has finished reading a file, Worker will send, via named pipe, to the parent process, summary statistics of the file containing the following information:</p>
<p>for each virus, in that country, it will send the number of cases per age category. For example, for a file / input_dir / China / 22-12-2020 containing records for SARS and COVID-19 will send summary statistics such as:</p>

```22-12-2020``` <br>
```China``` <br>
```SARS``` <br>
```Age range 0-20 years: 10 cases``` <br>
```Age range 21-40 years: 23 cases``` <br>
```Age range 41-60 years: 34 cases``` <br>
```Age range 60+ years: 45 cases``` <br>


```COVID-19``` <br>
```Age range 0-20 years: 20 cases``` <br>
```Age range 21-40 years: 230 cases``` <br>
```Age range 41-60 years: 340 cases``` <br>
```Age range 60+ years: 450 cases``` <br>

<p> If a Worker receives a SIGINT or SIGQUIT signal then it prints in a file named log_file.xxxx (where xxx is its process ID) the name of the countries (subdirectories) it manages, the total number of requests received by the parent process and the total number of requests answered successfully (ie there was no error in the query). </p>

__Logfile format:__
	
	
```Italy``` <br>
```China``` <br>
```Germany``` <br>
```TOTAL 29150``` <br>
```SUCCESS 25663``` <br>
```FAIL 3487``` <br>


<p> If a Worker receives a SIGUSR1 signal, it means that 1 or more new files have been placed in one of the subdirectories assigned to it. It will check subdirectories to find new files, read them and update the data structures it holds in memory. For each new file, it will send summary statistics to the parent process.</p>

<p> If a Worker process ends abruptly, the parent process will have to fork a new Worker process that will replace it. Therefore, the parent process should handle the SIGCHLD signal, as well as SIGINT and SIGQUIT. </p>

#### __The user will be able to give the following commands to the application:__

* */listCountries*

The application will print each country along with the process ID of the Worker process that manages its files. Example:

```Canada 123``` <br>
```Italy 5769``` <br>
```China 5770``` <br>

* */diseaseFrequency virusName date1 date2 [country]*

If no country argument is given, the application will print for the virusName disease the number of cases recorded in the system within the space [date1 ... date2]. If a country argument is given, the application will print for the virusName disease, the number of cases in the country that have been recorded in the period [date1 ... date2]. Example:
 <br>```153``` <br>

* */topk-AgeRanges k country disease date1 date2*

The application will print, for the country and the virus disease, the top k age categories that have shown cases of the specific virus in the specific country and their incidence rate. Example with k = 2: <br>
  ```20-30: 40% ```<br>
  ```60+: 20% ```<br>
  
 * */searchPatientRecord recordID*

The parent process forwards to all Workers the request via named pipes and waits for a response from the Worker with the record recordID. When he receives it, he prints it.
Example for someone who went to the hospital 23-02-2020 and left 28-02-2020: <br>

  ```776 Larry Jones SARS-1 87 23-02-2020 28-02-2020``` <br>
  
Example for someone who was admitted to the hospital on 23-02-2020 and has not yet been released: <br>

  ```776 Larry Jones SARS-1 87 23-02-2020 --``` <br>
  
 * */numPatientAdmissions disease date1 date2 [country]*

If a country argument is given, the application will print, on a new line, the total number of patients admitted to the hospital with the disease in that country within [date1 date2]. If no country argument is given, the application will print, for each country, the number of patients with the disease who were admitted to the hospital in the period [date1 date2]. Example: <br>

```Italy 153``` <br>
```Spain 769``` <br>
```France 570``` <br>

* */numPatientDischarges disease date1 date2 [country]*

If given the country argument, the application will print on a new line, the total number of patients with the disease who have been discharged from the hospital in that country within [date1 date2]. If no country argument is given, the application will print, for each country, the number of patients with the disease who have been discharged from the hospital in the period [date1 date2]. Example:

```Italy 153``` <br>
```Spain 769``` <br>
```France 570``` <br>

* */exit*

<p>Exit the application. The parent process sends a SIGKILL signal to the Workers, waits for them to terminate, and prints it to a file named log_file.xxxx where xxx is its process ID, the name of the countries (subdirectories) that participated in the application, the total number of requests received by the user and answered successfully and the total number of requests where an error occurred and / or were not completed. Before it terminates, it will properly release all the free memory. Example:</p>

 ```Italy``` <br>
 ```China``` <br>
 ```Germany``` <br>
 ```TOTAL 29150``` <br>
 ```SUCCESS 25663``` <br>
 ```FAIL 3487``` <br>
 
Implementation of named pipes: <br>
![](/images/syspro.PNG?raw=true "Implementation of named pipes")
 
 ### B. __The script create_infiles.sh (25%)__
 
<p> You will write a bash script that creates test subdirectories and input files that you will use to debug your program. Of course during the development of your program you can use a few small files to debug. The script create_infiles.sh works as follows: </p>

 <br> ```./create_infiles.sh diseasesFile countriesFile input_dir numFilesPerDirectory numRecordsPerFile```  <br>


- diseaseFile: a file with virus names (one per line) <br>
- countriesFile: a file with country names (one per line) <br>
- input_dir: the name of a directory where the subdirectories and input files will be placed <br>

## Implemantation techniques

* The pipes are blocking. Select () was not used.
The messages from the father are sent to one loop for each worker and read to another.
* All questions are processed from the workers and the results are sent to the father via named pipe, except / listCountries.
* For \topk a binary maxheap is implemented each time, where the path to the requested node is based on the binary representation of the number of the requested position. By isolating the appropriate digits each time we consider for 0 the left and 1 the right. Each node of the tree
contains the number of cases and the name of either the virus or the city and pointers to the right and left child, and to the father.
* For workers, records are stored in a red black tree, which has the complexity of searching and entering, both in the middle and in the worst case O (logn).
* When the father or children are writing or reading, the signals are blocked using sigprocmask.





