import OpenAI from "openai";
import { NextResponse } from "next/server";

type ConnectChatGptRequest = {
  apiKey?: string;
  model?: string;
};

export async function POST(request: Request) {
  try {
    const body = (await request.json()) as ConnectChatGptRequest;
    const apiKey = body.apiKey?.trim();
    const model = body.model?.trim() || "gpt-4.1-mini";

    if (!apiKey) {
      return NextResponse.json(
        { ok: false, message: "OpenAI API key is required." },
        { status: 400 },
      );
    }

    const client = new OpenAI({ apiKey });
    await client.models.retrieve(model);

    return NextResponse.json({
      ok: true,
      message: `ChatGPT connected successfully with model ${model}.`,
    });
  } catch (error) {
    const message =
      error instanceof Error ? error.message : "Unable to connect to ChatGPT.";

    return NextResponse.json({ ok: false, message }, { status: 400 });
  }
}
