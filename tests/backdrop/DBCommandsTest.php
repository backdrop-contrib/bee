<?php
/**
 * @file
 * PHPUnit tests for Bee Database commands.
 */

use PHPUnit\Framework\TestCase;
/**
 * Test Bee's database commands.
 */
class DBCommandsTest extends TestCase {
  /**
   * Make sure that the db-export command works.
   */
  public function test_db_export_command_works() {
    global $bee_test_root, $bee_test_backdrop_db_name;
    $output = shell_exec('bee db-export database.sql');
    $this->assertStringContainsString("The '$bee_test_backdrop_db_name' database was exported to '$bee_test_root/backdrop/database.sql.gz'.", (string) $output);
    $this->assertTrue(file_exists("$bee_test_root/backdrop/database.sql.gz"));
  }

  /**
   * Make sure that the db-import command works.
   */
  public function test_db_import_command_works() {
    global $bee_test_root, $bee_test_backdrop_db_name;
    $output = shell_exec('bee db-import database.sql.gz');
    $this->assertStringContainsString("'database.sql.gz' was imported into the '$bee_test_backdrop_db_name' database.", (string) $output);

    // Remove DB export for future tests.
    exec("rm $bee_test_root/backdrop/database.sql.gz");
  }

  /**
   * Make sure that the db-drop command works.
   */
  public function test_db_drop_command_works() {
    global $bee_test_root, $bee_test_backdrop_db_name;
    $output = shell_exec('bee db-export database.sql');
    $this->assertStringContainsString("The '$bee_test_backdrop_db_name' database was exported to '$bee_test_root/backdrop/database.sql.gz'.", (string) $output);
    $this->assertTrue(file_exists("$bee_test_root/backdrop/database.sql.gz"));

    $output = shell_exec('bee db-drop -y');
    $this->assertStringContainsString("The '$bee_test_backdrop_db_name' database was successfully dropped.", (string) $output);

    $output = shell_exec('bee db-import database.sql.gz');
    $this->assertStringContainsString("'database.sql.gz' was imported into the '$bee_test_backdrop_db_name' database.", (string) $output);

    // Remove DB export for future tests.
    exec("rm $bee_test_root/backdrop/database.sql.gz");
  }

  /**
   * Make sure that the db-query command works.
   */
  public function test_db_query_command_works() {
    $output = shell_exec('bee db-query "SELECT type, filename FROM {system} WHERE name = \'system\'"');
    $this->assertStringContainsString("module,core/modules/system/system.module", (string) $output);
  }
}
