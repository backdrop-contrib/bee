<?php
/**
 * @file
 * PHPUnit tests for Bee DBLog commands.
 */

use PHPUnit\Framework\TestCase;

class DBLogCommandsTest extends TestCase {

  /**
   * Make sure that the log command works.
   */
  public function test_log_command_works() {
    $output_all = shell_exec('bee log');
    $this->assertStringContainsString(' | Date', $output_all);
    $this->assertStringContainsString(' | Message', $output_all);

    $output_one = shell_exec('bee log 1');
    $this->assertStringContainsString('dblog module installed.', $output_one);

    exec('bee log --count=2', $output_count);
    // The header and trailing newline add 2 extra rows to the output.
    $this->assertEquals(4, count($output_count));

    $output_severity = shell_exec('bee log --severity=info');
    $this->assertStringContainsString(' | Info', $output_severity);
    $this->assertStringNotContainsString(' | Notice', $output_severity);

    $output_type = shell_exec('bee log --type=cron');
    $this->assertStringContainsString(' | cron', $output_type);
    $this->assertStringNotContainsString(' | system', $output_type);
  }

}
