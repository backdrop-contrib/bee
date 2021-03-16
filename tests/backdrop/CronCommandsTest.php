<?php
/**
 * @file
 * PHPUnit tests for Bee Cron commands.
 */

use PHPUnit\Framework\TestCase;

class CronCommandsTest extends TestCase {

  /**
   * Make sure that the cron command works.
   */
  public function test_cron_command_works() {
    $output = shell_exec('bee cron');
    $this->assertStringContainsString('Cron ran successfully.', $output);
  }

}
