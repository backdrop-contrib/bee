<?php
/**
 * @file
 * PHPUnit tests for Backdrop Console Cron commands.
 */

use PHPUnit\Framework\TestCase;

class CronCommandsTest extends TestCase {

  /**
   * Make sure that the cron command works.
   */
  public function test_cron_command_works() {
    $output = shell_exec('b cron');
    $this->assertStringContainsString('Cron ran successfully.', $output);
  }

}
