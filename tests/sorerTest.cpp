//lang::Cpp

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "../src/dataframe/dataframe.h"
#include "../src/dataframe/column/column.h"
#include "../src/utils/args.h"
#include "../src/store/key.h"
#include "../src/store/kvstore.h"

Arguments args;

/** Entrypoint for sorer program. Sorer used to read in data and then convert
 * it to a DataFrame */
int main(int argc, char **argv) {
	puts("Sorer test started...");
    Key* key = new Key("df", 0);
    KVStore* store = new KVStore(0, nullptr);
    DataFrame* d = DataFrame::fromFile(argv[1], key, store);
    //some checks to make sure our dataframe reading worked properly
    assert(d->ncols() == 12);
    assert(d->nrows() == 1000);
    assert(d->get_int(0, 0) == -1437879);
    assert(d->get_bool(1, 0) == false);
    assert(d->get_bool(1, 1) == true);
    assert(d->get_double(3, 0) - 1040.435425 < 0.00001);
    assert(strcmp(d->get_string(2, 0)->c_str(), "inflexive") == 0);
    assert(strcmp(d->get_string(2, 1)->c_str(), "abdominal's") == 0);
    assert(strcmp(d->get_string(2, 2)->c_str(), "discourtesy's") == 0);
    assert(d->get_int(4, 1) == -1924364);
    assert(strcmp(d->get_string(6, 143)->c_str(), "cornworms") == 0);

    // delete d;
    // delete s;
	puts("Sorer test passed!");
}