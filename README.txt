/* CSci4061 Fall 2018 Project 3
* Name: Nicole Walker, Kadin Schermers, Samuel Ball
* X500: walkx0760, scher528, ballx188 */


How to compile and run:
Compile by running make in the source directory.
Run by calling ./web_server with the correct arguments, and add requests by calling wget <request> in another terminal window.


How it works:
Dispatcher threads take requests and add them to a queue. Worker threads take requests out of this queue and service them, finding data either in-cache or on disk and returning it to the client. When data is not found in-cache but it is found on disk, the data gets added to the cache according to our cache replacement policy.


Caching mechanism:
We decided to use an array-queue implementation, where the index of the next cache slot to store data in gets iterated with each addition and loops around to the beginning of the array. We use an oldest-first replacement policy, meaning we can overwrite whatever is in the cache entry indexed by this iterator because it is the oldest-modified entry.


Contributions:
Sam: worker thread, cv & locking implementation
Kadin: cache functions, readfromdisk, main
Nikki: set-up git repo, dispatch, getcontenttype, main
All: Coding various segments (we didn't really keep track of who did exactly what, but we all contributed. We shared a git repo and would take turns editing and pushing changes - or simultaneously edit separate sections to merge - tackling issues and unfinished features until completion). Lots and lots and lots of debugging
