#ifndef __NODE_H__
#define __NODE_H__

// STL
#include <optional>
#include <stdexcept>
#include <string>

/**
 * @brief Exception thrown when attempting to access uninitialized node attributes.
 * 
 * This exception is thrown when trying to get optional node attributes (DCs, IXPs, etc.)
 * that have not been set yet.
 */
class NodeAttributeNotSetException : public std::runtime_error {
public:
  explicit NodeAttributeNotSetException(const std::string& attribute)
    : std::runtime_error(attribute + " has not been set yet.") {}
};

/**
 * @brief Class with the node information.
 *
 * The Node class is used to represent a Node in an Optical Fiber Network inside
 * the simulator.
 *
 * Nodes objects are needed, alongside Link objects, in order to create an
 * Optical Fiber Network architecture used as a base for the simulations.
 *
 * The Node class enfolds/contains two attributes:
 * -A mandatory/necessary attribute called Id (id) used as an unique identifier.
 * -An optional attribute called Label (label) used for adding extra information
 * for identifying the node.
 *
 * The Node class consists of 4 methods for setting/accessing the values of
 * their Id and Label attributes respectively.
 */

class Node {
 public:
  /**
   * @brief Constructs a new Node object that represents the default Node
   * allocated by calling the void constructor. This constructor sets the Id
   * attribute to -1. This constructor sets the Label to an empty string ("").
   *
   */
  Node(void);
  /**
   * @brief Copy constructor for Node objects.
   *
   * @param node The Node object to be copied.
   */
  Node(const Node &node);
  /**
   * @brief Constructs a new Node object that represents a Node with the given
   * Id number. This constructor sets the Id attribute to the value of the id
   * parameter given when invoking the constructor. This constructor sets the
   * Label to the value of the label parameter given when invoking the
   * constructor.
   *
   * @param id The desired Id (integer) number used to identify the Node.
   * @param label The desired Label (string) used to add extra information to
   * the Node.
   */
  Node(int id, std::optional<std::string> label);
  /**
   * @brief Constructs a new Node object that represents a Node with the given
   * Id number, data centers (DCs), Internet Exchange Points (IXPs), and label.
   *
   * @param id The desired Id (integer) number used to identify the Node.
   * @param dcs The number of data centers (DCs) associated with the Node.
   * @param ixps The number of Internet Exchange Points (IXPs) associated with the Node.
   * @param label The desired Label (string) used to add extra information to
   * the Node.
   */
  Node(int id, std::optional<int> dcs, std::optional<int> ixps, std::string label);
  /**
   * @brief Constructs a new Node object with all optional parameters.
   *
   * @param id The desired Id (integer) number used to identify the Node (required).
   * @param dcs Optional number of data centers (DCs) associated with the Node.
   * @param ixps Optional number of Internet Exchange Points (IXPs) associated with the Node.
   * @param population Optional population value associated with the Node.
   * @param label Optional Label (string) used to add extra information to the Node.
   * @param longitude Optional longitude coordinate for the Node's geographical location.
   * @param latitude Optional latitude coordinate for the Node's geographical location.
   * @param param1 Optional custom parameter 1 (double) for user-defined purposes.
   * @param param2 Optional custom parameter 2 (double) for user-defined purposes.
   */
  Node(int id, 
       std::optional<int> dcs = std::nullopt,
       std::optional<int> ixps = std::nullopt,
       std::optional<double> population = std::nullopt,
       std::optional<std::string> label = std::nullopt,
       std::optional<double> longitude = std::nullopt,
       std::optional<double> latitude = std::nullopt,
       std::optional<double> param1 = std::nullopt,
       std::optional<double> param2 = std::nullopt);
  /**
   * @brief Set the Id unique number of the current Node object.
   * This method is meant to be used on Nodes that were created as default Nodes
   * with no previous Id number given (and automatically set to -1).
   *
   * @param id The desired Id (integer) number used to identify the Node.
   */
  void setId(int id);
  /**
   * @brief Get the Id unique number of the current Node object.
   *
   * @return The Id (integer) unique number used to identify the Node.
   */
  int getId(void) const;
  /**
   * @brief Set the number of data centers (DCs) associated with the current Node object.
   *
   * @param dcs The desired number of data centers (DCs) associated with the Node.
   */
  void setDCs(std::optional<int> dcs);
  /**
   * @brief Get the number of data centers (DCs) associated with the current Node object.
   *
   * @return Optional number of data centers (DCs) associated with the Node.
   */
  std::optional<int> getDCs(void) const;
  /**
   * @brief Set the number of Internet Exchange Points (IXPs) associated with the current Node object.
   * 
   * @param ixps The desired number of Internet Exchange Points (IXPs) associated with the Node.
   */
  void setIXPs(std::optional<int> ixps);
  /**
   * @brief Get the number of Internet Exchange Points (IXPs) associated with the current Node object.
   *
   * @return Optional number of Internet Exchange Points (IXPs) associated with the Node.
   */
  std::optional<int> getIXPs(void) const;
  /**
   * @brief Set the longitude coordinate of the current Node object.
   *
   * @param longitude The desired longitude coordinate of the Node.
   */
  void setLongitude(std::optional<double> longitude);
  /**
   * @brief Get the longitude coordinate of the current Node object.
   *
   * @return Optional longitude coordinate of the Node.
   */
  std::optional<double> getLongitude(void) const;
  /**
   * @brief Set the latitude coordinate of the current Node object.
   *
   * @param latitude The desired latitude coordinate of the Node.
   */
  void setLatitude(std::optional<double> latitude);
  /**
   * @brief Get the latitude coordinate of the current Node object.
   *
   * @return Optional latitude coordinate of the Node.
   */
  std::optional<double> getLatitude(void) const;
  /**
   * @brief Set the population value of the current Node object.
   *
   * @param population The desired population value associated with the Node.
   */
  void setPopulation(std::optional<double> population);
  /**
   * @brief Get the population value of the current Node object.
   *
   * @return Optional population value associated with the Node.
   */
  std::optional<double> getPopulation(void) const;
  /**
   * @brief Set the optional Label string of the current Node object.
   *
   * @param label The desired string (std::string) used to label the Node for
   * additional information.
   */
  void setLabel(std::optional<std::string> label);
  /**
   * @brief Get the Label string of the current Node object.
   *
   * @return Optional Label string (std::string) of the Node.
   */
  std::optional<std::string> getLabel(void) const;

  /**
   * @brief Set the parameter 1 (double) of the current Node object.
   *
   * @param param1 The desired parameter 1 (double) of the Node.
   */
  void setParam1(std::optional<double> param1);
  /**
   * @brief Get the parameter 1 (double) of the current Node object.
   *
   * @return Optional parameter 1 (double) of the Node.
   */
  std::optional<double> getParam1(void) const;
  /**
   * @brief Set the parameter 2 (double) of the current Node object.
   *
   * @param param2 The desired parameter 2 (double) of the Node.
   */
  void setParam2(std::optional<double> param2);
  /**
   * @brief Get the parameter 2 (double) of the current Node object.
   *
   * @return Optional parameter 2 (double) of the Node.
   */
  std::optional<double> getParam2(void) const;

  /**
   * @brief Set the degree of the current Node object.
   * This method is meant to be used when building the adjacency list
   *
   * @param degree The desired degree of the Node.
   */
  void setDegree(int degree);
  /**
   * @brief Get the degree of the current Node object.
   *
   * @return Degree of the Node.
   */
  int getDegree(void) const;

 private:
  int id;
  std::optional<int> dcs;
  std::optional<int> ixps;
  std::optional<double> longitude;
  std::optional<double> latitude;
  std::optional<double> population;
  std::optional<double> param1;
  std::optional<double> param2;
  std::optional<std::string> label;

  // Degree is innitialized to -1
  // and then set when building the adjacency list
  int degree = -1;
};

#endif