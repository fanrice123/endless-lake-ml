#include "mldata.h"

int main(int argc, char *argv[])
{
    auto&& [X, Y] = load_data(argv[1], argv[2]);
    auto&& [train_data, train_labels, cv_data, cv_labels] = cv_split(X, Y, 0.4);

    
    return 0;
}
