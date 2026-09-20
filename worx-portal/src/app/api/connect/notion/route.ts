import { Client } from "@notionhq/client";
import { NextResponse } from "next/server";

type ConnectNotionRequest = {
  token?: string;
  databaseId?: string;
};

export async function POST(request: Request) {
  try {
    const body = (await request.json()) as ConnectNotionRequest;
    const token = body.token?.trim();

    if (!token) {
      return NextResponse.json(
        { ok: false, message: "Notion token is required." },
        { status: 400 },
      );
    }

    const notion = new Client({ auth: token });
    const me = await notion.users.me({});

    if (body.databaseId?.trim()) {
      await notion.databases.retrieve({ database_id: body.databaseId.trim() });
    }

    return NextResponse.json({
      ok: true,
      message: "Notion connected successfully.",
      workspaceUser: "name" in me ? me.name : null,
    });
  } catch (error) {
    const message =
      error instanceof Error ? error.message : "Unable to connect to Notion.";

    return NextResponse.json({ ok: false, message }, { status: 400 });
  }
}
