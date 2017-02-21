# QML Movie Renderer

This application demonstrates how it is possible to render Qt Quick content into a series of images which can later be combined into a Movie file.

## Usage
You need to set:
 - a target QML file (ideally one that animates on its own)
 - Image output size
 - Duration of animation
 - Frames per second
 - Directory to output images in
 - Prefix to the output filenames
 - Image format
 
Once all necessary fields are filled, the "Render Movie" button should enable itself
 
When you press the "Render Movie" button two progress bars will be display in the status bar.
The first indicates the progress of how many frames have been rendered by Qt Quick.
The second indicates the progress of writing the frames to disk in the desired image format.
 
Once the rendering process is completed, the output directory selected should have a series of image files. Use these images files to generate a video or moving picture.  For example with ffmpeg:

`ffmpeg -r 60 -f image2 -s 1280x720 -i %d.jpg -vcodec libx264 -crf 25 -pix_fmt yuv420p hello_world_60.mp4`
