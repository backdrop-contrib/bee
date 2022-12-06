<?php
/**
 * @file
 * PHPUnit tests for Bee Update commands.
 */

use PHPUnit\Framework\TestCase;
/**
 * Test Bee's command for updating the Backdrop database.
 */
class UpdateCommandsTest extends TestCase {

  /**
   * Make sure that the update-db command works.
   */
  public function test_update_db_command_works() {
    // Install an older version of Devel.
    exec('cd modules && wget -q https://github.com/backdrop-contrib/devel/releases/download/1.x-1.5.5/devel.zip');
    exec('cd modules && unzip devel.zip && rm devel.zip');
    exec('bee en --y devel');

    // Make sure there are no database updates.
    $output_clean = shell_exec('bee update-db');
    $this->assertStringContainsString('There are no pending database updates.', (string) $output_clean);

    // Update Devel to a newer version.
    exec('cd modules && rm -rf devel && wget -q https://github.com/backdrop-contrib/devel/releases/download/1.x-1.6.0/devel.zip');
    exec('cd modules && unzip devel.zip && rm devel.zip');

    // Perform database updates.
    $output_updates = shell_exec('bee update-db --y');
    $this->assertStringContainsString('Remove option for Krumo skin.', (string) $output_updates);
    $this->assertStringContainsString('Would you like to apply all pending updates?', (string) $output_updates);
    $this->assertStringContainsString('All pending updates applied.', (string) $output_updates);

    // Cleanup Devel.
    exec('bee disable --y devel');
    exec('bee uninstall --y devel');
    exec('rm -rf modules/devel');
  }

}
