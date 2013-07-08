#include "ros/ros.h"
#include "std_msgs/String.h"
#include <navigationISL/hotspot.h>
#include "navigationISL/neighborInfo.h"
#include <iostream>
#include <ctime>
#include <sstream>

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#include <QDebug>
#include <qjson/parser.h>
#include <QDir>
#include <QFile>

double mu;
bool startGeneratingHotspot = false;


// Received start info
void startInfoCallback(navigationISL::neighborInfo neighborInfo)
{
    QString str = QString::fromStdString(neighborInfo.name);

    if(str == "start")
    {
        startGeneratingHotspot = true;
    }

    return;
}

// Reads the config file
bool readConfigFile(QString filename)
{
    QFile file(filename);

    if(!file.exists()) return false;

    if(!file.open(QFile::ReadOnly)) return false;

    QJson::Parser parser;

    bool ok;

    QVariantMap result = parser.parse(&file,&ok).toMap();

    if(!ok){

        file.close();
        qDebug()<<"Fatal reading error";

        return false;
    }
    else
    {

        mu  = result["poissonMU"].toDouble();

        qDebug()<< " mu " << mu;//result["poissonMU"].toString();

    }
    file.close();
    return true;

}


int main(int argc, char **argv)
{

  ros::init(argc, argv, "hotspotobserverISL");

  ros::NodeHandle n;

  ros::Publisher hotspotPublisher = n.advertise<navigationISL::hotspot>("hotspotobserverISL/hotspot", 1000);

  ros::Subscriber startInfoSubscriber = n.subscribe("communicationISL/neighborInfo",5, startInfoCallback);


  ros::Rate loop_rate(1);

  QString path = QDir::homePath();
  path.append("/fuerte_workspace/sandbox/configISL.json");


  if(!readConfigFile(path)){

      qDebug()<< "Read Config File Failed!!!";

      ros::shutdown();

      return 0;
  }


//  int count = 0;

//  int hotspotThreshold = 0.005*RAND_MAX;

  const gsl_rng_type * T;
  gsl_rng * r;
  gsl_rng_env_setup();
  T = gsl_rng_default;
  r = gsl_rng_alloc(T);
  int prn;

  while (ros::ok())
  {
      if (startGeneratingHotspot)
      {
          prn = gsl_ran_poisson(r, mu);
          if(prn>=1){

              std::time_t t = std::time(0);  // t is an integer type

              //std::cout << t ;
              //std::cout << " prn "<< prn << " time " << t;

              qDebug()<< " time " << t;

              navigationISL::hotspot hs;

              hs.hotspot = t;


              // ROS_INFO("%s", msg.data.c_str());

              hotspotPublisher.publish(hs);
          }
      }

  /*
      if(rand()%RAND_MAX < hotspotThreshold){

          std::time_t t = std::time(0);  // t is an integer type

          std::cout << t ;

         navigationISL::hotspot hs;

         hs.hotspot = t;


   // ROS_INFO("%s", msg.data.c_str());

        hotspotPublisher.publish(hs);
    }
  */
    ros::spinOnce();

    loop_rate.sleep();
   // ++count;
  }


  return 0;
}



