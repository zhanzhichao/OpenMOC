#include "Surface.h"

short int Surface::_n = 0;

static int auto_id = 10000;

/**
 * @brief Returns an auto-generated unique surface ID.
 * @details This method is intended as a utility mehtod for user's writing
 *          OpenMOC input files. The method makes use of a static surface
 *          ID which is incremented each time the method is called to enable
 *          unique generation of monotonically increasing IDs. The method's
 *          first ID begins at 10000. Hence, user-defined surface IDs greater
 *          than or equal to 10000 is prohibited.
 */
int surf_id() {
    int id = auto_id;
    auto_id++;
    return id;
}


/**
 * @brief Constructor assigns unique ID and user-defined ID for a surface.
 * @details Assigns a default boundary condition for this surface to 
 *          BOUNDARY_NONE.
 * @param id the user-defined surface id
 */
Surface::Surface(const short int id){

    if (id >= surf_id())
        log_printf(ERROR, "Unable to set the ID of a surface to %d since "
		 "surface IDs greater than or equal to 10000 is probibited "
		 "by OpenMOC.", id);

    _id = id;
    _uid = _n;
    _n++;
    _boundary_type = BOUNDARY_NONE;
}


/**
 * @brief Destructor.
 */
Surface::~Surface() { }


/**
 * @brief Return the surface's unique ID.
 * @return the surface's unique ID
 */
short int Surface::getUid() const {
    return _uid;
}


/**
 * @brief Return the surface's user-defined id.
 * @return the surface's user-defined id
 */
short int Surface::getId() const {
    return _id;
}


/**
 * @brief Return the type of surface (ie, XPLANE, CIRCLE, etc).
 * @return the surface type
 */
surfaceType Surface::getSurfaceType() {
    return _surface_type;
}


/**
 * @brief Returns the type of boundary conditions for this surface (REFLECTIVE, 
 *        VACUUM or BOUNDARY_NONE)
 * @return the type of boundary type for this surface
 */
boundaryType Surface::getBoundaryType(){
    return _boundary_type;
}


/**
 * @brief Sets the boundary condition type (ie, VACUUM or REFLECTIVE) for this
 *        surface.
 * @param boundary_type the boundary condition forthis surface
 */
void Surface::setBoundaryType(boundaryType boundary_type) {
    _boundary_type = boundary_type;
}


/**
 * @brief Return true or false if a point is on or off of a surface.
 * @param point pointer to the point of interest
 * @return on (true) or off (false) the surface
 */
bool Surface::onSurface(Point* point) {

    /* Uses a threshold to determine whether the point is on the surface */
    if (abs(evaluate(point)) < ON_SURFACE_THRESH)
        return true;
    else
        return false;
}


/**
 * @brief Return true or false if a localcoord is on or off of a surface.
 * @param coord pointer to the localcoord of interest
 * @return on (true) or off (false) the surface
 */
bool Surface::onSurface(LocalCoords* coord) {
    return onSurface(coord->getPoint());
}


/**
 * @brief Constructor.
 * @param id the surface id
 * @param A the first coefficient in \f$ A * x + B * y + C = 0 \f$
 * @param B the second coefficient in \f$ A * x + B * y + C = 0 \f$
 * @param C the third coefficient in \f$ A * x + B * y + C = 0 \f$
 */
Plane::Plane(const short int id, const double A, const double B,
             const double C): 
    Surface(id) {
        _surface_type = PLANE;
        _A = A;
        _B = B;
        _C = C;
}


/**
 * @brief Returns the minimum x value of -INFINITY on this surface.
 * @return the minimum x value of -INFINITY
 */
double Plane::getXMin(){
    log_printf(ERROR, "Plane::getXMin() not yet implemented");
    return -std::numeric_limits<double>::infinity();
}


/**
 * @brief Returns the maximum x value of INFINITY on this surface.
 * @return the maximum x value of INFINITY
 */
double Plane::getXMax(){
    log_printf(ERROR, "Plane::getXMax() not yet implemented");
    return std::numeric_limits<double>::infinity();
}


/**
 * @brief Returns the minimum y value of -INFINITY on this surface.
 * @return the minimum y value of -INFINITY
 */
double Plane::getYMin(){
    log_printf(ERROR, "Plane::getYMin not yet implemented");
    return -std::numeric_limits<double>::infinity();
}


/**
 * @brief Returns the maximum y value of INFINITY on this surface.
 * @return the maximum y value of INFINITY
 */
double Plane::getYMax(){
    log_printf(ERROR, "Plane::getYMax not yet implemented");
    return std::numeric_limits<double>::infinity();
}


/**
* @brief Finds the intersection point with this plane from a given point and
*        trajectory defined by an angle.
* @param point pointer to the point of interest
* @param angle the angle defining the trajectory in radians
* @param points pointer to a point to store the intersection point
* @return the number of intersection points (0 or 1)
*/
inline int Plane::intersection(Point* point, double angle, Point* points) {

    double x0 = point->getX();
    double y0 = point->getY();

    int num = 0;                        /* number of intersections */
    double xcurr, ycurr;        /* coordinates of current intersection point */

    /* The track is vertical */
    if ((fabs(angle - (M_PI / 2))) < 1.0e-10) {

        /* The plane is also vertical => no intersections */
        if (_B == 0)
            return 0;

        /* The plane is not vertical */
        else {
            xcurr = x0;
            ycurr = (-_A * x0 - _C) / _B;
            points->setCoords(xcurr, ycurr);
        
            /* Check that point is in same direction as angle */
            if (angle < M_PI && ycurr > y0)
                num++;
            else if (angle > M_PI && ycurr < y0)
                num++;
            return num;
        }
    }

    /* If the track isn't vertical */
    else {
        double m = sin(angle) / cos(angle);

        /* The plane and track are parallel, no intersections */
        if (fabs(-_A/_B - m) < 1e-11 && _B != 0)
            return 0;

        else {
            xcurr = -(_B * (y0 - m * x0) + _C) / (_A + _B * m);
            ycurr = y0 + m * (xcurr - x0);
            points->setCoords(xcurr, ycurr);

            if (angle < M_PI && ycurr > y0)
                num++;
            else if (angle > M_PI && ycurr < y0)
                num++;
            
            return num;
        }
    }
}


/**
 * @brief Converts this plane's attributes to a character array.
 * @details The character array returned conatins the type of plane (ie,
 *          PLANE) and the A, B, and C coefficients in the
 *          quadratic surface equation.
 * @return a character array of this plane's attributes
 */
std::string Plane::toString() {
    std::stringstream string;

    string << "Surface id = " << _id << ", type = PLANE " << ", A = "
           << _A << ", B = " << _B << ", C = " << _C;

    return string.str();
}


/**
 * @brief Prints a string representation of all of the plane's objects to
 *        the console.
 */
void Plane::printString() {
    log_printf(RESULT, toString().c_str());
}


/**
 * @brief Constructor for a plane perpendicular to the x-axis.
 * @param id the user-defined surface id
 * @param x the location of the plane along the x-axis
 */
XPlane::XPlane(const short int id, const double x): 
    Plane(id, 0, 1, -x) {
        _surface_type = XPLANE;
        _x = x;
}


/**
 * @brief Set the location of this xplane on the x-axis.
 * @param x the location of the xplane on the x-axis
 */
void XPlane::setX(const double x) {
    _x = x;
}


/**
 * @brief Returns the location of the xplane on the x-axis.
 * @return the location of the xplane on the x-axis
 */
double XPlane::getX() {
    return _x;
}


/**
 * @brief Returns the minimum x value on the xplane.
 * @return the minimum x value
 */
double XPlane::getXMin(){
    return _x;
}


/**
 * @brief Returns the maximum x value on the xplane.
 * @return the maximum x value
 */
double XPlane::getXMax(){
    return _x;
}


/**
 * @brief Returns the minimum y value of -INFINITY on the xplane.
 * @return the minimum y value of -INFINITY
 */
double XPlane::getYMin(){
    return -std::numeric_limits<double>::infinity();
}


/**
 * Returns the maximum y value of INFINITY on this xplane.
 * @return the maximum y value of INFINITY
 */
double XPlane::getYMax(){
    return std::numeric_limits<double>::infinity();
}


/**
 * @brief Converts this xplane's attributes to a character array.
 * @details The character array returned conatins the type of plane (ie,
 *          XPLANE) and the A, B, and C coefficients in the
 *          quadratic surface equation and the location of the plane on
 *          the x-axis.
 * @return a character array of this xplane's attributes
 */
std::string XPlane::toString() {
    std::stringstream string;

    string << "Surface id = " << _id << ", type = XPLANE " << ", A = "
           << _A << ", B = " << _B << ", C = " << _C << ", x = " << _x;

    return string.str();
}


/**
 * @brief Constructor for a plane perpendicular to the y-axis.
 * @param id the surface id
 * @param y the location of the plane along the y-axis
 */
YPlane::YPlane(const short int id, const double y): 
    Plane(id, 1, 0, -y) {
        _surface_type = YPLANE;
        _y = y;
}


/**
 * @brief Set the location of this yplane on the y-axis.
 * @param y the location of the yplane on the y-axis
 */
void YPlane::setY(const double y) {
    _y = y;
}


/**
 * @brief Returns the location of the yplane on the y-axis.
 * @return the location of the yplane on the y-axis
 */
double YPlane::getY() {
    return _y;
}

/**
 * @brief Returns the minimum x value of -INFINITY on this yplane.
 * @return the minimum x value of -INFINITY
 */
double YPlane::getXMin(){
    return -std::numeric_limits<double>::infinity();
}


/**
 * @brief Returns the maximum x value of INFINITY on this yplane.
 * @return the maximum x value of INFINITY
 */
double YPlane::getXMax(){
    return std::numeric_limits<double>::infinity();
}


/**
 * @brief Returns the minimum y value on this yplane.
 * @return the minimum y value
 */
double YPlane::getYMin(){
    return _y;
}


/**
 * @brief Returns the maximum y value on this yplane.
 * @return the maximum y value
 */
double YPlane::getYMax(){
    return _y;
}


/**
 * @brief Converts this yplane's attributes to a character array
 * @details The character array returned conatins the type of plane (ie,
 *          YPLANE) and the A, B, and C coefficients in the
 *          quadratic surface equation and the location of the plane on
 *          the y-axis.
 * @return a character array of this yplane's attributes
 */
std::string YPlane::toString() {
    std::stringstream string;
        
    string << "Surface id = " << _id << ", type = YPLANE " << ", A = "
           << _A << ", B = " << _B << ", C = " << _C << ", y = " << _y;

    return string.str();
}


/**
 * @brief Prints a string representation of all of the yplane's objects to
 *        the console.
 */
void YPlane::printString() {
    log_printf(RESULT, toString().c_str());
}


/**
 * @brief Constructor for a plane perpendicular to the z-axis.
 * @param id the surface id
 * @param z the location of the plane along the z-axis
 */
ZPlane::ZPlane(const short int id, const double z): 
    Plane(id, 0, 0, -z) {
        _surface_type = ZPLANE;
        _z = z;
}


/**
 * @brief Set the location of this zplane on the z-axis.
 * @param z the location of the zplane on the z-axis
 */
void ZPlane::setZ(const double z) {
    _z = z;
}


/**
 * @brief Returns the location of the zplane on the z-axis.
 * @return the location of the zplane on the z-axis
 */
double ZPlane::getZ() {
    return _z;
}


/**
 * @brief Returns the minimum x value of -INFINITY on this zplane.
 * @return the minimum x value of -INFINITY
 */
double ZPlane::getXMin(){
    return -std::numeric_limits<double>::infinity();

}


/**
 * @brief Returns the maximum x value of INFINITY on this zplane.
 * @return the maximum x value of INFINITY
 */
double ZPlane::getXMax(){
    return std::numeric_limits<double>::infinity();
}


/**
 * @brief Returns the minimum y value of -INFINITY on this zplane.
 * @return the minimum y value of -INFINITY
 */
double ZPlane::getYMin(){
    return -std::numeric_limits<double>::infinity();
}


/**
 * @brief Returns the maximum y value on this zplane.
 * @return the maximum y value
 */
double ZPlane::getYMax(){
    return std::numeric_limits<double>::infinity();
}

/**
 * @brief Converts this zplane's attributes to a character array.
 * @details The character array returned conatins the type of plane (ie,
 *          ZPLANE) and the A, B, and C coefficients in the
 *          quadratic surface equation and the location of the plane along
 *          the z-axis.
 * @return a character array of this zplane's attributes
 */
std::string ZPlane::toString() {
    std::stringstream string;

    string << "Surface id = " << _id << ", type = ZPLANE " << ", A = "
           << _A << ", B = " << _B << ", C = " << _C << ", z = " << _z;

    return string.str();
}


/**
 * @brief Prints a string representation of all of the zplane's objects to
 *        the console.
 */
void ZPlane::printString() {
    log_printf(RESULT, toString().c_str());
}


/**
 * @brief constructor.
 * @param id the surface id
 * @param x the x-coordinte of the circle center
 * @param y the y-coordinate of the circle center
 * @param radius the radius of the circle
 */
Circle::Circle(const short int id, const double x, const double y, 
               const double radius): 
    Surface(id) {
        _surface_type = CIRCLE;
        _A = 1.;
        _B = 1.;
        _C = -2.*x;
        _D = -2.*y;
        _E = x*x + y*y - radius*radius;
        _radius = radius;
        _center.setX(x);
        _center.setY(y);
}


/**
 * @brief Return the x-coordinate of the circle's center point.
 * @return the x-coordinate of the circle center
 */
double Circle::getX0() {
    return _center.getX();
}


/**
 * @brief Return the y-coordinate of the circle's center point.
 * @return the y-coordinate of the circle center
 */
double Circle::getY0() {
    return _center.getY();
}


/**
 * @brief Finds the intersection point with this circle from a given point and
 *        trajectory defined by an angle (0, 1, or 2 points).
 * @param point pointer to the point of interest
 * @param angle the angle defining the trajectory in radians
 * @param points pointer to a an array of points to store intersection points
 * @return the number of intersection points (0 or 1)
 */
int Circle::intersection(Point* point, double angle, Point* points) {

    double x0 = point->getX();
    double y0 = point->getY();
    double xcurr, ycurr;
    int num = 0;                        /* Number of intersection points */
    double a, b, c, q, discr;

    /* If the track is vertical */
    if ((fabs(angle - (M_PI / 2))) < 1.0e-10) {

        /* Solve for where the line x = x0 and the surface F(x,y) intersect
         * Find the y where F(x0, y) = 0
         * Substitute x0 into F(x,y) and rearrange to put in
         * the form of the quadratic formula: ay^2 + by + c = 0
         */
        a = _B * _B;
        b = _D;
        c = _A * x0 * x0 + _C * x0 + _E;
      
        discr = b*b - 4*a*c;
        
        /* There are no intersections */
        if (discr < 0)
            return 0;

        /* There is one intersection (ie on the surface) */
        else if (discr == 0) {
            xcurr = x0;
            ycurr = -b / (2*a);
            points[num].setCoords(xcurr, ycurr);
            if (angle < M_PI && ycurr > y0)
                num++;
            else if (angle > M_PI && ycurr < y0)
                num++;
            return num;
        }
    
        /* There are two intersections */
        else {
            xcurr = x0;
            ycurr = (-b + sqrt(discr)) / (2 * a);
            points[num].setCoords(xcurr, ycurr);
            if (angle < M_PI && ycurr > y0)
                num++;
            else if (angle > M_PI && ycurr < y0)
                num++;
        
            xcurr = x0;
            ycurr = (-b - sqrt(discr)) / (2 * a);
            points[num].setCoords(xcurr, ycurr);
            if (angle < M_PI && ycurr > y0)
                num++;
            else if (angle > M_PI && ycurr < y0)
                num++;
            return num;
        }
    }

    /* If the track isn't vertical */
    else {
        /* Solve for where the line y-y0 = m*(x-x0) and the surface F(x,y) 
         * intersect. Find the (x,y) where F(x, y0 + m*(x-x0)) = 0
         * Substitute the point-slope formula for y into F(x,y) and 
         * rearrange to put in the form of the quadratic formula: 
         * ax^2 + bx + c = 0
         */
        double m = sin(angle) / cos(angle);
        q = y0 - m * x0;
        a = _A + _B * _B * m * m;
        b = 2 * _B * m * q + _C + _D * m;
        c = _B * q * q + _D * q + _E;

        discr = b*b - 4*a*c;
        
        /* There are no intersections */
        if (discr < 0)
            return 0;

        /* There is one intersection (ie on the surface) */
        else if (discr == 0) {
            xcurr = -b / (2*a);
            ycurr = y0 + m * (points[0].getX() - x0);
            points[num].setCoords(xcurr, ycurr);
            if (angle < M_PI && ycurr > y0)
                num++;
            else if (angle > M_PI && ycurr < y0)
                num++;
            return num;
        }

        /* There are two intersections */
        else {
            xcurr = (-b + sqrt(discr)) / (2*a);
            ycurr = y0 + m * (xcurr - x0);
            points[num].setCoords(xcurr, ycurr);
            if (angle < M_PI && ycurr > y0) {
                num++;
            }
            else if (angle > M_PI && ycurr < y0) {
                num++;
            }

            xcurr = (-b - sqrt(discr)) / (2*a);
            ycurr = y0 + m * (xcurr - x0);
            points[num].setCoords(xcurr, ycurr);
            if (angle < M_PI && ycurr > y0) {
                num++;
            }
            else if (angle > M_PI && ycurr < y0) {
                num++;
            }

            return num;
        }
    }
}


/**
 * @brief Converts this circle's attributes to a character array.
 * @details The character array returned conatins the type of plane (ie,
 *          CIRCLE) and the A, B, C, D and E coefficients in the
 *          quadratic surface equation.
 * @return a character array of this circle's attributes
 */
std::string Circle::toString() {
    std::stringstream string;

    string << "Surface id = " << _id << ", type = CIRCLE " << ", A = "
           << _A << ", B = " << _B << ", C = " << _C << ", D = " << _D
           << ", E = " << _E << ", x0 = " << _center.getX() << ", y0 = " 
           << _center.getY() << ", radius = " << _radius;
    
    return string.str();
}


/**
 * @brief Prints a string representation of all of the circle's objects to
 *        the console.
 */
void Circle::printString() {
    log_printf(RESULT, toString().c_str());
}


/**
 * @brief Returns the minimum x value on this circle.
 * @return the minimum y value
 */
double Circle::getXMin(){
    return _center.getX() - _radius;
}


/**
 * @brief Returns the maximum x value on this circle.
 * @return the maximum x value
 */
double Circle::getXMax(){
    return _center.getX() + _radius;
}

/**
 * @brief Returns the minimum y value on this circle.
 * @return the minimum y value
 */
double Circle::getYMin(){
    return _center.getY() - _radius;
}


/**
 * @brief Returns ths maximum y value on this circle.
 * @return the maximum y value
 */
double Circle::getYMax(){
    return _center.getY() + _radius;
}
