<?php
/**
 * @file
 * PHPUnit tests for Backdrop Console Install commands.
 */

use PHPUnit\Framework\TestCase;

class InstallCommandsTest extends TestCase {

  /**
   * Download Backdrop.
   *
   * This code is run *once* before any of the following tests.
   */
  public static function setUpBeforeClass(): void {
    exec('mysql -u root -e "CREATE DATABASE IF NOT EXISTS install_test; GRANT ALL PRIVILEGES ON install_test.* TO \'backdrop\'@\'%\' IDENTIFIED by \'backdrop\';"');
  }

  /**
   * Make sure that the install command works.
   */
  public function test_install_command_works() {

  }

}
