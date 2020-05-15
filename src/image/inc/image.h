#pragma once

#include <cassert>
#include <cstring>
#include <cmath>

#include <algorithm>
#include <string>
#include <vector>
#include <stdexcept>

using namespace std;


class Image
{

private:


public:

  // fields
  int w, h, c;
  float* data;

  /**
   * @brief Constructs a new Image object
   * 
   */
  Image();


  /**
   * @brief Constructs a new Image object
   * 
   * @param w width of the image
   * @param h height of the image
   * @param c number of channels for the image
   */
  Image(int w, int h, int c=1);
  

  /**
   * @brief Destroy the Image object
   * 
   */
  ~Image();
  

  /**
   * @brief Copy Constructor
   * 
   * @param other the image for which we are copying into the current instance
   */
  Image(const Image& other);
  

  /**
   * @brief Move Constructor
   * 
   * @param other the image for which to move into the current instance
   */
  Image(Image&& other);


  /**
   * @brief copy assignment
   * 
   * @param other the image being copied
   * @return Image& a reference to this instance that has the data copied from other image
   */
  Image& operator=(const Image& other);
  

  /**
   * @brief move assignment
   * 
   * @param other the image being moved
   * @return Image& a reference to this instance that has the data moved from the other image
   */
  Image& operator=(Image&& other);


  /**
   * @brief compares whether the two images have the same data
   * 
   * @param other the other image to compare this image to
   * @return true if the two images have the same data
   * @return false if the two images are different 
   */
  bool operator==(Image& other);


  /**
   * @brief operator overload to access/set pixel value
   * which allows access to a pixel value like:
   * px_value = im(x, y, c);
   * and setting of a pixel as such:
   * im(x, y, c) = px_value;
   * 
   * @param x the x position of the pixel
   * @param y the y position of the pixel
   * @param ch the channel of the pixel
   * @return float& a reference to the pixel value
   */
  float& operator()(int x, int y, int ch);


  /**
   * @brief operator overload to access/set pixel value in the first channel
   * allows access to a pixel value like:
   * px_value = im(x, y);
   * and setting of a pixel as such:
   * im(x, y) = px_value;
   * 
   * @param x the x position of the pixel
   * @param y the y position of the pixel
   * @return float& a reference to the pixel value
   */
  float& operator()(int x, int y);
  

  /**
   * @brief operator overload to access pixel value but does 
   * not allow modification of pixel value
   * 
   * @param x the x position of the pixel
   * @param y the y position of the pixel
   * @param ch the channel of the pixel
   * @return const float& a reference to the pixel value
   */
  const float& operator()(int x, int y, int ch) const;
  

  /**
   * @brief operator overload to access pixel value but does
   * not allow modification of pixel value
   * allows access to a pixel value like:
   * px_value = im(x, y);
   * 
   * @param x the x position of the pixel
   * @param y the y position of the pixel
   * @return const float& a reference to the pixel value
   */
  const float& operator()(int x, int y) const;


  /**
   * @brief gets/sets a pixel value
   * 
   * @param x the x position of the pixel
   * @param y the y position of the pixel
   * @param ch the channel of the pixel
   * @return float& a reference to the pixel value
   */
  float& pixel(int x, int y, int ch);

  
  /**
   * @brief gets a pixel value
   * 
   * @param x the x position of the pixel
   * @param y the y position of the pixel
   * @param ch the channel of the pixel
   * @return const float& a reference to the pixel value
   */
  const float& pixel(int x, int y, int ch) const;


  /**
   * @brief Gets/sets a pixel value in the first channel
   * 
   * @param x the x position of the pixel
   * @param y the y position of the pixel
   * @return float& a reference to the pixel value
   */
  float& pixel(int x, int y);


  /**
   * @brief gets a pixel value in the first channel
   * 
   * @param x the x position of the pixel
   * @param y the y position of the pixel
   * @return const float& a reference to the pixel value
   */
  const float& pixel(int x, int y) const;
  
  
  /**
   * @brief Get the pixel value at the given point and channel
   * 
   * @param x the x position of the pixel
   * @param y the y position of the pixel
   * @param ch the channel of the pixel
   * @return float the pixel value
   */
  float get_pixel(int x, int y, int ch) const;
  

  /**
   * @brief Get the pixel object at the given point in the first channel
   * 
   * @param x the x position of the pixel
   * @param y the y position of the pixel
   * @return float the pixel value
   */
  float get_pixel(int x, int y) const;
  

  /**
   * @brief Set the pixel object at the given point and channel
   * 
   * @param x the x position of the pixel
   * @param y the y position of the pixel
   * @param ch the channel of the pixel
   * @param v the value for which to set the given pixel
   */
  void set_pixel(int x, int y, int ch, float v);

  
  /**
   * @brief Gets a pointer to a row of pixels in the given channel
   * 
   * @param row the row desired in the image
   * @param ch the channel of the row desired
   * @return const float* a float pointer to an array representing the pixel values of the row
   */
  const float* RowPtr(int row, int ch) const;


  /**
   * @brief Gets a pointer to a row of pixels in the given channel
   * 
   * @param row the row desired in the image
   * @param ch the channel of the row desired
   * @return float* a float pointer to an array representing the pixel values of the row
   */
  float* RowPtr(int row, int ch);
  

  /**
   * @brief checks whether a given coordinate point exists in the image
   * 
   * @param x the x position
   * @param y the y position
   * @return true if the coordinate is within the image bounds
   * @return false if the coordinate falls outside the image bounds
   */
  bool contains(float x, float y) const;


  /**
   * @brief Checks if the given pixel point is empty/blank/black
   * 
   * @param x the x position
   * @param y the y position
   * @return true if the given pixel is blank
   * @return false if the given pixel has a non zero value
   */
  bool is_empty(int x, int y) const;
  

  /**
   * @brief checks that a given patch of size 2*w centred at (x,y) is non-empty
   * 
   * @param x the x position
   * @param y the y position
   * @param w the offset from (x,y) for which to check in all directions
   * @return true if the image contains some value/content within the patch
   * @return false if the patch is blank
   */
  bool is_nonempty_patch(int x, int y, int w=0) const;

  /**
   * @brief Creates a resized image using nearest neighbour interpolation
   * 
   * @param w the new width of the image
   * @param h the new height of the image
   * @return Image a new resized image
   */
  Image nn_resize(int w, int h);


  /**
   * @brief Creates a resized image using nearest bilinear interpolation
   * 
   * @param w the new width of the image
   * @param h the new height of the image
   * @return Image a new resized image
   */
  Image bilinear_resize(int w, int h);


  /**
   * @brief Gets the value pixel a given floating point coordinate using nearest neighbour interpolation
   * 
   * @param x the x coordinate 
   * @param y the y coordinate
   * @param ch the channel from which to retrieve the pixel value
   * @return float the value of the pixel which the coordinate maps to using nearest neighbour
   */
  float nn_interpolate(float x, float y, int ch) const;


  /**
   * @brief Gets the value of a pixel given floating point cooridnates using bilinear interpolation of the 
   * surrounding pixels
   * 
   * @param x the x coordinate 
   * @param y the y coordinate
   * @param ch the channel from which to retrieve the pixel value
   * @return float the value of the pixel which the coordinate maps to using nearest neighbour
   */
  float bilinear_interpolate(float x, float y, int ch) const;
  

  /**
   * @brief gets the size of the image in terms of number of pixels
   * 
   * @return int the size of the image in number of pixels
   */
  int size() const;


  /**
   * @brief clears the image setting all pixel values to 0
   * 
   */
  void clear() const;


  /**
   * @brief 
   * 
   * @return Image 
   */
  Image abs() const;
  

  // member functions for modifying image

  /**
   * @brief normalizes the values in the image to all sum to 1
   * handy for covolutions and creating a filter. 
   * 
   */
  void l1_normalize();


  /**
   * @brief Converts the given image's colourspace from rgb to hsv
   * 
   */
  void RGBtoHSV();


  /**
   * @brief Converts the given image's colourspace from hsv to rgb 
   * 
   */
  void HSVtoRGB();


  /**
   * @brief Converts the given image's colourspace from lch to rgb
   * 
   */
  void LCHtoRGB();


  /**
   * @brief Converts the given image's colourspace from rgb to lch
   * 
   */
  void RGBtoLCH();


  /**
   * @brief Shifts every pixel in the given channel but the given value
   * 
   * @param ch the channel to target
   * @param v the value by which to add to each pixel
   */
  void shift(int ch, float v);


  /**
   * @brief 
   * 
   * @param ch 
   * @param v 
   */
  void scale(int ch, float v);


  /**
   * @brief 
   * 
   * @param thres 
   */
  void threshold(float thres);


  /**
   * @brief Clamps the values in the pixels to be bounded between 0 and 1
   * 
   */
  void clamp();


  /**
   * @brief 
   * 
   */
  void feature_normalize();


  /**
   * @brief 
   * 
   */
  void feature_normalize_total();
  

  /**
   * @brief Get the channel object
   * 
   * @param c 
   * @return Image 
   */
  Image get_channel(int c) const;


  /**
   * @brief Set the channel object
   * 
   * @param c 
   * @param im 
   */
  void set_channel(int c,const Image& im);


  /**
   * @brief converts the current image to a new image which is 
   * a grayscaled version. Requries that the current image 
   * have 3 channels for (r, g, b)
   * 
   * @return Image a grayscaled copy of the current image
   */
  Image rgb_to_grayscale() const;


  /**
   * @brief 
   * 
   * @return Image 
   */
  Image transpose() const;
  

  /**
   * @brief Serializes the data in this image
   * 
   * @param file the path to the file for which to serialize the data to
   */
  void save(const string& file);
  
  
  /**
   * @brief Deserializes the data in a file into an image
   * 
   * @param file the path to the file for which to try deserialize the data from
   * @return Image the resulting image from the file
   */
  static Image load(const string& file);

};

Image load_image(const string& filename);
void save_png(const Image& im, const string& name);
void save_image(const Image& im, const string& name);