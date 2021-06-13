#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;
    client.call(srv);
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{
    
    int white_pixel = 255;
  //   ROS_INFO("in process_image_callback");
    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera

    int img_height = img.height;
    int img_width = img.width;
  //  printf("the img height width:%d,%d\n",img_height,img_width);
    int numWhitePixel = 0;
    int totalPixels = img_height*img_width;
    int whitePos = 0;
    for(int i=0;i<img_height;i++){
        for(int j=0;j<img_width;j++)
        {
            // here use index of row * step, to get to the position of 
            // start of row data, so finally I already traverse all of
            // the picture data
            int index = i*img.step+j*3;
            int red_channel = img.data[index];
            int green_channel = img.data[index+1];
            int blue_channel = img.data[index+2];
            if(red_channel==255 && green_channel==255 && blue_channel==255)
            {
                numWhitePixel++;
                whitePos += j;
            }
        }
    }
   // printf("whitePos and num pixel:%d,%d\n",whitePos,numWhitePixel);
    if(numWhitePixel==0)
    {
        drive_robot(0.0,0.0);
        return;
    }
    int pos_average = whitePos / numWhitePixel;
    if(pos_average <= img_width/3)
    {
        drive_robot(0.0,0.5);
    }
    else if(pos_average >= 2*img_width/3)
    {
        drive_robot(0.0,-0.5);
    }
    else
    {
        float ratio = (float)( 1.0f * numWhitePixel / totalPixels );
        printf("The ratio is:%f\n",ratio);
        if(ratio >= 0.5f)
        {
            //already near enough,then stop
            drive_robot(0.0,0.0);
        }
        else
        {
            drive_robot(0.5,0.0);
        }
        
        
    }
    return;

}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}