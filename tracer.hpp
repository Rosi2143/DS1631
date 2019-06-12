/**
 * @file tracer.hpp
 * @author Michael Rossner (Schrott.Micha@web.de)
 * @brief 
 * @version 0.1
 * @date 2019-06-12
 * 
 * @copyright Copyright (c) 2019
 * MIT license - see license file
 */

class tracer
{
    tracer(int logLevel){_logLevel=logLevel;}
    ~tracer();

    bool checkLoglevel(int logLevel){
            if(logLevel>_logLevel)
            { 
                return true;
            }
            else
            {
                    return false;
            }
    }
    void setLogLevel(int logLevel){_logLevel=logLevel;}
            
private:
    int _logLevel;
};