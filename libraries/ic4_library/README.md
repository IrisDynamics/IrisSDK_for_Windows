# IrisControls4Library
Library repository for all deployment platforms and supported development chips

This repo includes two types of files: general files, and platform specific files.

General files include anything relating to the IC4 API such as the commands listed on the Iris Wiki and the parser for such commands.

Platform specific files are those that are directly responsible for the packaging and sending of commands as well as any additional overhead required by a specific chip or chipset. 

For example, parseCommand() is a function that will live in a general header/file while the function check() which calls parseCommand will live in a platform specific header/file. Each platform will have a file with a check() in it but how check works is dependant on how the platform functions. As such many of the platform specific functions will exisit in a pure virtual form and by including the correct header for the development platform of your choice, the function will be overwritten. 

## GUI Run Loop Formatting

    void run() {  

            check();
            
            switch(gui_frame_state){
            
                ////////////// RX STATE /////////////////////////////////
                case rx: {
                                    
                    /////////////////// IRIS CONTROLS IS TIMED OUT ////////////////////
                    if ( is_timed_out() ) {
                    
                        // DO THINGS WHEN TIMED OUT...
                        
                        set_disconnected();
                    }
                    break;
                } // RX CASE
                
                ////////////// TX STATE /////////////////////////////////
                case tx: {
    
                    /////////////////// IRIS CONTROLS RECONNECT AFTER TIMEOUT /////////
                    if ( restored_connection()) {
                        print_l("\r\r==== Iris Controls Was Timed Out ====\r\r");
                        
                        //DO THINGS...
                        
                    }
        
                    /////////////////// IRIS CONTROLS HAS ESTABLISHED CONNECTION //////
                    if (new_connection()) {
                    
                        //DO THINGS UPON NEW CONNECTION
                        
                    }              
        
                    /////////////////// IRIS CONTROLS REGULAR UPDATES /////////////////
                    if (is_connected() ) {        

                        uint32_t tnow = system_time();
                        static uint32_t gui_timer = 0;
                        if ( (uint32_t)(tnow - gui_timer) > gui_update_period ) {
                            gui_timer = tnow;        
                           
                           //DO THINGS WHEN CONNECTED
                           
                            end_of_frame();
                        }
                    }
                    send();
                    break;
                } // TX CASE             
            } // SWITCH     
        } // RUN
