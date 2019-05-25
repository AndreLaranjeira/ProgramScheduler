#include "data_structures.h"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE("Table can be created and deleted", "[scheduler_table]"){

  scheduler_table *table = NULL;

  REQUIRE( create_table(&table) == SUCCESS );
  REQUIRE( table->count == 0 );
  REQUIRE( table->last_job == -1 );
  REQUIRE( table->next_alarm != 0 );
  REQUIRE( table->first == NULL);
  REQUIRE( table->last == NULL);
  REQUIRE( table->next == NULL);

  REQUIRE( delete_table(&table) == SUCCESS );
}

TEST_CASE("Table can add one or more elements", "[scheduler_table]"){

  scheduler_table *table = NULL;
  table_item item;
  time_t aux = time(NULL);
  item.start_time = aux;

  REQUIRE( create_table(&table) == SUCCESS );

  REQUIRE( add_table_item(table, item) == SUCCESS );
  REQUIRE( table->count == 1 );
  REQUIRE( table->last_job == 0 );
  REQUIRE( table->next_alarm == aux );
  REQUIRE( table->first != NULL);
  REQUIRE( table->last != NULL);
  REQUIRE( table->next != NULL);
  REQUIRE( table->first == table->last);
  REQUIRE( table->first == table->next);
  REQUIRE( table->first != &item);

  REQUIRE( add_table_item(table, item) == SUCCESS );
  REQUIRE( table->count == 2 );
  REQUIRE( table->last_job == 1 );
  REQUIRE( table->first != NULL);
  REQUIRE( table->last != NULL);
  REQUIRE( table->next != NULL);
  REQUIRE( table->first != table->last);
  REQUIRE( table->first == table->next);
  REQUIRE( table->last != table->next);
  REQUIRE( table->first->next == table->last);
  REQUIRE( table->first->job == 0);
  REQUIRE( table->first->next->job == 1);

  REQUIRE( add_table_item(NULL, item) == INVALID_ARG );

  REQUIRE( delete_table(&table) == SUCCESS );

}
