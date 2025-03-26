extern int SCORE;

class Square {
    public:

        // attributes
        float squareX;
        float squareY;
        float squareZ;
        bool active;

        // constructor
        Square(float x, float y, float z, bool isActive){
            squareX = x;
            squareY = y;
            squareZ = z;
            active = isActive;
        }

        // setters
        void setX(float x) {squareX = x;} 
        void setY(float y) {squareY = y;} 
        void setZ(float z) {squareZ = z;} 
        void setIsActive(bool isActive) {active = isActive;}

        //getters
        float getX() {return squareX;}
        float getY() {return squareY;}
        float getZ() {return squareZ;}
        bool getActive() {return active;}


        bool checkCollisionTarget(Square targetSquare, float squareX, float squareY, float circleX, float circleY){

            float circleRadius = 0.5f;
        
            glm::vec2 center(circleX + circleRadius, circleY + circleRadius);
        
            // targetSquares have a width of 1.0f and height of 1.0f
        
            float targetX = targetSquare.getX(); 
            float targetY = targetSquare.getY();
        
            glm::vec2 aabb_half_extents(0.5f, 0.5f);
            glm::vec2 aabb_center(
                targetX + aabb_half_extents.x, 
                targetY + aabb_half_extents.y
            );
        
            glm::vec2 difference = center - aabb_center;
            glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
        
            glm::vec2 closest = aabb_center + clamped;
        
            difference = closest - center;
        
            if(glm::length(difference) < circleRadius){
                SCORE += 1;
            }
        
            return glm::length(difference) < circleRadius;
        
        }
        
};