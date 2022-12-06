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
    $output = shell_exec('bee db-export database.sql');
    $this->assertStringContainsString("The 'backdrop' database was exported to '/app/backdrop/database.sql.gz'.", (string) $output);
    $this->assertTrue(file_exists('/app/backdrop/database.sql.gz'));
  }

  /**
   * Make sure that the db-import command works.
   */
  public function test_db_import_command_works() {
    $output = shell_exec('bee db-import database.sql.gz');
    $this->assertStringContainsString("'database.sql.gz' was imported into the 'backdrop' database.", (string) $output);

    // Remove DB export for future tests.
    exec('rm /app/backdrop/database.sql.gz');
  }

}
