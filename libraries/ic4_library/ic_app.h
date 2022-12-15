/**
 * @file ic_app.h
 * @author Kyle Hagen <khagen@irisdynamics.com>
 * @version 2.2.0
    
    @copyright Copyright 2022 Iris Dynamics Ltd 
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

    For questions or feedback on this file, please email <support@irisdynamics.com>.
 */

#ifndef SRC_IRISCONTROLS_RTAPP_H_
#define SRC_IRISCONTROLS_RTAPP_H_


#include "io_elements.h"


/**
 * Iris Controls apps are classes that contain functions that are automatically called by AppChain objects.
 * This allows a programmer to build an app's setup and shutdown behavior (ie what sliders and buttons etc
 * are populated on IrisControls when the app starts), as well as program the run function of the app
 * (ie what does this app do repeatedly). The app object is added to an AppChain - potentially along with other
 * apps - and these functions will be called by the app chain.
 *
 * The setup function is called when the app is added to an appchain, or when the appchains setup function is called
 * The shutdown function is called when the app is removed, or the appchain's shutdown function is called.
 * Typically, apps are added to a chain when either a connection to IrisControls is detected (the USB is plugged in or
 * the app is started), or when a user presses a button or does something that would provoke new functionality
 * from the device.
 *
 * These apps are primarily intended to be used by GUI functionalities. AppChain objects run functions should
 * typically be called once every 10ms or slower to prevent excessive amounts of linked-list iterations and
 * conditional checking often associated with running the gui.
 *
 * These objects also contain a parser function that can be called by the main IrisControls parser. This is
 * accomplished again through an AppChain object. The Appchain contains a parse function which iterates its
 * linked list of app objects, calling their parse functions in turn. For this functionality to work,
 * the Appchain must be assigned to one of the AppChain pointers built into the IrisControls object.
 * When IrisControls runs its parser, it checks for valid appchains assigned to its pointers and calls
 * their parse functions (which call the parse functions of any apps within the appchain's linked list).
 */
class ICApp {
	public:

	virtual ~ICApp (){};

	/**
	 * Typically commands PC app to display some sliders or other information
	 * Also initializes anything required to run the app
	 */
	virtual void setup() {
	}
	/**
	 * Must hide any GUI element displayed in the setup as IrisControls does not
	 * logically group elements are hide them on its own.
	 */
	virtual void shutdown() {
	}
	/**
	 * Will be called frequently. Should not block or take too many clock cycles
	 * Break up larger tasks into multiple apps or use state machines and iterate
	 * only once per call
	 */
	virtual void run() = 0;
	/**
	 * Should return 1 if the cmd was recognized
	 */
	virtual int parse( char * cmd, char * args ) { return 0; }

	ICApp * next = 0;

	FlexButton * launch_button = 0;

protected:

};


/**
 * A one-way linked list of apps
 * Does not delete apps or do any dynamic memory allocation
 * Allows a single call to the setup(), shutdown() or run() function to be applied to every app in the chain
 * Deleting the chain shuts down every app in the chain, but does not delete that app
 *
 * the error field will have bits set if the app trying to be added or removed already exists in the chain
 */
class AppChain {
  public:

	enum ERROR_TYPES {
		add_error,
		remove_error,
	};
	static u32 errors;

	/**
	 * When the appchain is deleted, all of the apps within it have their shutdown functions called.
	 * The apps within the appchain are not deleted when the appchain is deleted
	 */
	~AppChain () {
		ICApp * p = head;
		while (p) {
			p->shutdown();
			p = p->next;
		}
	}
	/**
	 * Add the passed app into the linked list and call its setup function
	 */
    void add (ICApp * new_app, int quiet = 0) {
    	if (contains(new_app)) {
    		errors |= (1<<add_error);
    		return;
    	}
    	new_app->next = head;
    	head = new_app;
		if ( !quiet ) new_app->setup();
    }
    /**
     * Returns true when the passed app is contained in the appchains linked list
     */
    int contains (ICApp * search_for) {
    	ICApp * p = head;
		while (p) {
			if (p == search_for) return 1;
			p = p->next;
		}
		return 0;
	}
    /**
     * Remove the app from the linked list and call the shutdown function for that app
     * Does not delete the app object
     */
	int remove (ICApp * to_remove) {
		if (!to_remove) 	{}
		else if (!head) 	{}
		else {
			ICApp * p = head;

			if (head == to_remove) {
				head = head->next;
				p->shutdown();
				return 1;
			}
			while (p) {
				if (p->next == to_remove) {
					p->next->shutdown();
					p->next=p->next->next;
					return 1;
				}
				p = p->next;
			}
		}

		errors |= (1<<remove_error);
		return 0;
	}
	/**
	 * Iterates the linked list and calls the startup function for each app
	 */
    void setup() {
    	for (ICApp * p = head; p; p = p->next) p->setup();
    }
    /**
     * Iterates the linked list and calls the run function for each app
     */
    void run() {
      for (ICApp * p = head; p; p = p->next) p->run();
    }
    /**
     * Iterates the linked list and calls the parse function for each app
     */
    int parse( char * cmd, char * args) {
    	int ret = 0;
    	for (ICApp * p = head; p; p = p->next) ret |= p->parse(cmd,args);
    	return ret;
    }

  protected:
    /// Linked list head
    ICApp * head = 0;
};


#endif /* SRC_IRISCONTROLS_RTAPP_H_ */
