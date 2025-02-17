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
        
};