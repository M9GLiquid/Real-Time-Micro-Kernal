# Real-Time Micro-Kernal

Project
V
T
20
2
3
-
Real
-
Time Micro
-
Kernel
Project
2023
-
0
3
-
05
Authors: Hieu Tran, Thomas Lundqvist
Supervisor: Hazem Ali
, Ove Andersson
Group: 9
Course:
Computer System Engineering I
I
(
DT401
3
)
Halmstad
University
2
Abstract
This project aims to work with an incomplete Real
-
Time Micro
-
Kernel (RTMK),
which we students were to complete with the knowledge of programming in C
language using IAR Embedded Workbench IDE.
A microkernel is the micro amount of softw
are that provides the mechanisms
needed to implement an operating system (OS)
[1]
. This project aims to
understand better what operating systems do and how tasks in an operating
system work and could be managed in different states for whether a
specific task
was running or waiting. The focus of this project will be inter
-
communication
between tasks and running the tasks asynchronously and synchronously and also
memory efficiency.
This report includes an introduction section with details about wha
t type of
operating system is used in this project and the role of an operating system in
general and how it is used in embedded systems, operation of the RTMK section
that details the implemented OS works with the help of illustrations Flow Chart
Diagrams
, assisting libraries section to fulfill the project requirements, a testing
section that includes each function implemented in the assisting libraries and
integration tests of different functions, conclusion section where we discuss the
design of the proj
ect and the code built for memory efficiency and lastly a
reference section that lists the sources of any material we have used for this
report.
A significant operating system will constantly evolve for several reasons and
contributes benefits for users a
nd developers, but it also has its limitations.
3
Contents
Abstract
................................
................................
................................
................................
................
2
1.
Introduction
................................
................................
................................
............................
5
2.
Operation of the Real
-
Time Micro
-
Kernel
................................
................................
....
6
3. Assisting Libraries
................................
................................
................................
.......................
8
3.1 Doubly Linked List Functions
................................
................................
............................
8
3.2 Declaration functions
................................
................................
................................
...........
8
3.3 Insertion and Decoupler Functions
................................
................................
................
8
3.4 isMember function
................................
................................
................................
................
9
3.5 Mail functions
................................
................................
................................
.........................
9
4. Testing
................................
................................
................................
................................
...........
10
4.1 Unit testing
................................
................................
................................
............................
10
4.2 Integration testing
................................
................................
................................
..............
12
5. Conclusion
................................
................................
................................
................................
....
13
References
................................
................................
................................
................................
.........
14
4
5
1.
Introduction
This project is part of the course "Computer System Engineering II" for
engineering students at Halmstad University.
An operating system is essential for the embedded system due to the system
typically has limited resources, such as memory and processing
power
[2]
. The
operating system coordinates its use among the various application programs for
various users. The so
-
called micro
-
kernel contains the most used functions in the
OS and is in the OS's main memory. Using an operating system will allow
multipl
e applications to run on the same hardware without interfering with each
other by managing the resources efficiently, provided that the tasks are not
running asynchronously
[3]
. Generally, an operating system can provide an
effective and standardized platf
orm for software development and execution in
an embedded system. It allows developers to focus on creating applications that
meet the system's specific needs
[2]
.
In this document, we will describe the implementation of a Real
-
Time Micro
-
Kernel in C progr
amming language along with the hardware programmable logic
controller (PLC) consisting of the CPU microchip SAM3X8E for a Real
-
time
operating system (RTOS) which is built for systems that require deterministic
and immediate responses to external events, su
ch as control systems and
medical devices.
To achieve the requirements and understand how the RTMK works, we were
given lectures, testing codes, and scheduled supervision. This project is divided
into three categories; each must be fulfilled to finish th
e RTMK project. These
three categories were Task Administration for task management creation,
running and terminating tasks, Inter
-
Process Communication for establishing
communication between running tasks synchronously and asynchronously, and
Timing funct
ions for managing time in the operating system. To fulfill these
functions, assisting libraries to create and manage two types of Doubly Linked
Lists are required. The first type of Linked List is related to Task Administration:
ReadyList
,
WaitingList
, and
Timerlist
. The second type of Linked List is related to
Inter
-
Process Communication which is the mailbox. In this report, we will discuss
further the implementation of RTMK, and the categories mentioned above.
6
2.
Operation of the Real
-
Time Micro
-
Kerne
l
The Kernel has two modes,
Initialization
and
Running
. In the initialization phase, the code initiates
and creates the
ReadyList
,
WaitingList
, and
TimerList
. The mailboxes follow suit and are created
and initialized, ending with the tasks to be run
constructed. Following this, Ticks is set to zero, and
the mode is set to running mode. We also dedicate the Next task before Loading the first context,
followed by the operational task running before they terminate themselves.
Handling Time was done by three functions,
wait
,
TimerInt
, and
set_deadline
.
wait is an elementary function. It suspends the task and switches to the following task/context.
The task can subsequentially only be woken up by the
TimerInt
function, which che
cks whether any
tasks in
WaitingList
or
TimerList
are overdue or ready to return to work.
set_deadline
is used to reschedule a task and then change to the first task/context in the queue if
changing the deadline has altered the queue.
7
Our task messaging is divided into four functions,
receive_no_wait
,
receive_wait
,
send_wait
, and
send_no_wait
.
send_wait
belongs to the synchronous part because it blocks a task if it is sending, indicating it
needs to wait before it can resume. If there
is a waiting task, the message is delivered, and the
task becomes unblocked. Otherwise, the sending task is blocked until a task is available.
receive_wait
tries to collect a message from a mailbox. If there is a message waiting for it, it will
be collecte
d. If there is no message waiting, the receiving task will be blocked. If the message is a
"
send_wait
" type, the sending task will unblock the task.
send_no_wait
and
receive_no_wait
fall under the asynchronous umbrella as it does not block the
calling tas
k and allows it to continue execution without blocking it.
send_no_wait
will send a message to the mailbox and allow the sender to continue their work
without waiting for someone to receive it.
receive_no_wait
will attempt to receive a message from the m
ailbox.
8
3
.
Assisting Libraries
Beside the functions and main code that were given in this project, new features
also needed to be added to fulfill the project requirements and making it simpler
to manipulate linked lists.
3.1 Doubly Linked List
Functions
Since this project would focus on the work of a doubly linked list, the code
needed to be built around this library. During the work with a linked list that can
insert and remove objects from specific lists for this project in a certain way, we
n
eeded to customize the functions created adapted for managing tasks for task
administration and messages for inter
-
process communication. For example,
instead of creating an insertion function that inserts an object at the end, a
sorting insertion function
is added to sort the inserted nodes after the deadline to
easier having access to the list with the lowest deadline.
3.
2
Declaration functions
In this project, two declaration functions were added. One of the functions took a
void as an argument and decla
red a list to create LinkedLists for the Task
Administration requirement; the other function took three arguments. It would
decorate a node containing its previous and next node, a task, message, and
TCnt. In both declaration functions, the calloc function
was used instead of the
malloc function due to the malloc does not set the memory to zero. In contrast,
calloc sets the memory allocated to zero.
3.3
Insertion and
Decoupler
Function
s
We wanted the list created to be sorted for the insertion function whenever a
new object is inserted. Therefore, the
addAtFirstSmaller
function was created.
Simply what the
addAtFirstSmaller
function does is compare the deadlines of the
inserted object and
sort it correctly in the list. The head and tail will be pointed
to the inserted node if the list is empty. Nevertheless, if the node or the list
inserted is empty, the function will return NULL.
To decouple an object from the list of two functions is im
plemented. These
functions are the decoupler function and an extract function. The decoupler
function takes a list and an object as arguments. The application of the decoupler
function is to disconnect the inserted node from the inserted list and return th
e
decoupled node, while the extract function only decouples the head node from
the list.
9
3.4 isMember function
While building the code for the project, multiple problems occurred where
checking whether a specific object existed in a specific list was need
ed. To solve
this issue, we created the
isMember
function, which took both the list and a node
as arguments. This function searches through the inserted list to match the node
in the argument. Another reason to use the
isMember
function is to avoid
creatin
g unnecessary code.
3.
5
Mail functions
During the project, new functions were required to be implemented, specified for
a message and a mailbox. The functions implemented are similar to those
mentioned earlier, such as the insertion, decouple, and
declaration functions,
except the new functions implemented, were customized for a message and a
mailbox.
10
4. Testing
4.1 Unit testing
The purpose of the unit testing is to ensure that each component works outside
the project's scope.
The
different aspects we decided to unit test were the mail and double
-
linked list
components.
To start it off, we first tested the double
-
linked list as it was the most critical to
the project start, and without it, the mail component would not work. To test
it,
we set up a few cases to try against.
4.1.1
AddAtFirstSmallest
was the first function to test, and we started by sending a
node into an empty list to see that it would stick. After that, we started testing
with different deadlines as the function was supposed to sort it by the deadline,
and we had a few minor issues
in the beginning with it sorting it wrongly, but
those were quickly fixed with a few more statements in the code sections to sort
it correctly.
4.1.2
When we had the sorting done, we could move into extraction and decoupling of
the node, and this was more
straightforward as the cases food it to work was
simple. the extract function should extract the head of the list sent in and then
call the decoupling. We later had to add a little extra to be sure not to remove
the Idle_Task as it had done that once befo
re. And to be sure it wouldn’t do it
again, we added a small statement never to let it happen again.
4.1.3
The last of the Doubly linked list to be tested, except for the create_list and
create_node, was the decoupler function which was to decouple a node
from the
list wherever it might be in the list and stitch the list back together. This is so we
can move the node into a different list if needed, which was required for the
entire project to function, and later in integration testing, we will go into dep
th.
11
4.1.4
With the decoupler, we knew we would need to check if the mail was a member
of the mailbox, and thus we created the need for an isMember function, and thus
it needed to be tested.
We sent in several cases to check whether the mail was in the b
eginning, middle,
or at the end of the mailbox and what would happen if there was no mail or only
one mail.
4.1.5
For the mailbox and its corresponding mail, we also had an extract and decoupler
specific to the mailbox, and the same unit test was applied.
The main difference
was that there would be no Idle_Task at the end of the mailbox, so there was no
need for the extra statement as with the doubly linked list extract function. was
not needed. With adding messages to the mailbox, we encountered the probl
em
of keeping track of the different types of messages, and I will go more in
-
depth
about this in the following subsection.
4.1.6
AddMessage was a little trickier as it was supposed to keep track of the number
of messages in the mailbox and the blocked me
ssages, which we could get to
function easily. Hence, it ended up being moved outside the function instead. A
few trials and errors with different statements were made to try and make the
blocked messages counter to decrement as it was removed until an eas
ier
method was established to have it outside of the function entirely.
4.1.7
Lastly, we tested our functions that would remove or add a node to the list to
ensure we had no memory leak or failure due to memory management. We
added a test case to accompl
ish these circumstances where a while statement
would repeat until it failed or continued forever. We had, for example, one issue
with our send_wait function where it would remove the mail from the mailbox
but not free it, thus creating a memory leak that
later on destroyed the system.
12
4.2 Integration testing
For the integration testing, we test to ensure that the different components can
work well together and that there are no hiccups between the different
components. To do this, we would focus on the test cases we received from Labs
1, 2, and 3. The testing
was performed using a combination of manual and
automated testing techniques, allowing for comprehensive software testing.
4.2.1
Inter
-
Process Communication was a different case where code was supposed to
be working together asynchronously and synchronou
sly in different parts and
dealing with the tasks in the code, which took more work to follow. Furthermore,
issues began to pop up one after the other, solving one revealed the next, and so
on, with careful use of the IDE's breakpoint system and the genera
l knowledge of
what the code should do to solve most of the issues.
4.2.2
In the last phase, Timer Functions implemented a timing aspect to the tasks
where the tasks had to leave the blocked list if it was time or if it was overdue to
leave. In this
phase, everything worked as it should except for a problem
regarding the mailbox and removing mail from the middle of the list. It turned
out that a piece of code had been copy
-
pasted and not changed, making the
code not work. The solution was to rework th
e remove function for the mailbox
list. This added a new case for our unit testing, which would solve the issue.
4.2.3
The testing methodology involved designing and executing test cases that
focused on the interactions between different components of the system. The
test cases were designed to cover a range of scenarios and inputs. They were
executed in a test environment
that simulated the actual production
environment. For example, one test case involved testing the integration
between the send_wait and receive_wait components. These test cases included
a range of scenarios, such as receiving before sending and sending a
nd never
receiving. The test was executed multiple times, and the results were analyzed
to ensure that the integration between the two components worked correctly in
all scenarios.
13
5. Conclusion
In summary, this document has focused on implementing a Real
-
Time Micro
-
Kernel (RTMK) with the assistance of three phases Task Administration, Inter
-
Process communication, and Timing Functions. Since a doubly linked list has
been essential to complete this project, it is also one of the most focused topics
discusse
d in this document. The insertion function built for the doubly linked list
prioritizes tasks based on their deadlines and examines them to sort them to the
correct position. This document has also discussed the role of operating systems
in general and why
it is needed in embedded systems.
One of the main priorities of this project is to manage the execution and the
state of the tasks scheduled in the doubly linked lists.
Several issues occurred during the work of the Real
-
Time Micro
-
Kernel project.
The is
sues we encountered were related to memory management, compiler
handler, and pointer management of the objects.
Most of the issues on the project were because of the pointer management,
which could result in the program ending up in a dummy handler or a n
ever
-
ending loop in a specific function. The compiler issue could be solved using a
volatile datatype because the program could not read the correct type of
datatype for the specific variable.
14
References
[1]
GeeksForGeeks.
Microkernel in Operating Systems.
https://www.geeksforgeeks.org/microkernel
-
in
-
operating
-
systems/
. (2023).
Accessed:
2023
-
0
2
-
28
[2]
Mahdi, Fazeli. Introduction to Operating Systems, 2023. Accessed: 2023
-
0
3
-
04.
[3]
Tammy, Noergaard. Demystifying Embedded
Systems Middleware, 2010.
Accessed: 2023
-
0
3
-
0
4
.
DT4013_Gr9.pdf
05 of 14
