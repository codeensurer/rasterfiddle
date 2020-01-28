/* 
 * This application demonstrates how to read an input text file of addresses and creates a MapInfo TAB file.
 * The input file for this sample is hardcoded, has 4 columns containing the first and last line of an address
 * and the latitude and logitude of their locations. The data is comma delimeted as follows:
 * 	 StreetAddress,MainAddress,X,Y
 * 	 3630 SPINNAKER DR,"ANCHORAGE, AK  99516-3429",-149.829305,61.10065
 * 	 9177 JAMES BLVD,"JUNEAU, AK  99801-9617",-134.585619,58.37361
 * 	 10821 SUSHANA CIR,"EAGLE RIVER, AK  99577-8394",-149.528306,61.318491
 * 	 1624 HILTON AVE,"FAIRBANKS, AK  99701-4018",-147.753998,64.838329
 * 	 824 BROOKS AVE,"SOLDOTNA, AK  99669-8204",-151.125283,60.475465
 * 	 520 PINE AVE,"KENAI, AK  99611-7557",-151.274853,60.569625
 * 	 342 HOLLAND CIR,"KODIAK, AK  99615-7039",-152.362316,57.814804
 * 	 2750 US HIGHWAY 278 E,"HOKES BLUFF, AL  35903-7608",-85.916208,33.993934
 * 	 531 E LAKESIDE DR,"FLORENCE, AL  35630-4118",-87.618445,34.813344
 * 	 5612 11TH AVE S,"BIRMINGHAM, AL  35222-4138",-86.745331,33.527431
 * 	 2811 MONROE ST,"SELMA, AL  36701-7843",-87.016163,32.365909
 * 	 621 LANIER PL,"TUSCALOOSA, AL  35406-3049",-87.561172,33.225632
 * 	 12694 BEAR CREEK RD,"DUNCANVILLE, AL  35456-2536",-87.491348,33.117867
 *
 * This sample looks for the input file named "InputText.csv" which is part of this project.
 * The file is opened from the current directory so if debugging it is best to set the current
 * directory to $(ProjectDir). Also, EFAL.DLL must be found on the system path. If this is not 
 * already the case, the path can be modified in the debug tab of the project properties.
*/